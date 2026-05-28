// Copyright 2025 RavBot Contributors
// SPDX-License-Identifier: Apache-2.0

#include "ravbot/licensing/license_manager.hpp"

#include <algorithm>
#include <chrono>
#include <cctype>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <regex>
#include <iostream>
#include <sstream>

#ifndef _WIN32
#include <unistd.h>
#endif

#include <openssl/evp.h>

#include "ravbot/config.hpp"
#include "ravbot/platform/process.hpp"

namespace ravbot::licensing {
namespace {

constexpr const char* kLicenseSecret = "Rtp@20080513";
constexpr const char* kLicenseServer = "http://120.197.43.148:9981";

std::string trim(std::string value) {
  auto is_space = [](unsigned char c) { return std::isspace(c) != 0; };
  value.erase(value.begin(),
              std::find_if(value.begin(), value.end(),
                           [&](char c) { return !is_space(c); }));
  value.erase(std::find_if(value.rbegin(), value.rend(),
                           [&](char c) { return !is_space(c); })
                  .base(),
              value.end());
  return value;
}

std::string uppercase(std::string value) {
  for (char& c : value) {
    c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
  }
  return value;
}

std::string sha256_hex(const std::string& input) {
  unsigned char digest[EVP_MAX_MD_SIZE];
  unsigned int digest_len = 0;

  EVP_MD_CTX* ctx = EVP_MD_CTX_new();
  if (!ctx) {
    return "";
  }
  if (EVP_DigestInit_ex(ctx, EVP_sha256(), nullptr) != 1 ||
      EVP_DigestUpdate(ctx, input.data(), input.size()) != 1 ||
      EVP_DigestFinal_ex(ctx, digest, &digest_len) != 1) {
    EVP_MD_CTX_free(ctx);
    return "";
  }
  EVP_MD_CTX_free(ctx);

  std::ostringstream out;
  out << std::hex << std::setfill('0');
  for (unsigned int i = 0; i < digest_len; ++i) {
    out << std::setw(2) << static_cast<int>(digest[i]);
  }
  return uppercase(out.str());
}

std::string run_command_trimmed(const std::string& command) {
  auto result = platform::exec_capture(command, 10);
  if (result.exit_code != 0) {
    return "";
  }
  return trim(result.output);
}

std::vector<std::string> read_lines(const std::filesystem::path& path) {
  std::ifstream in(path);
  std::vector<std::string> lines;
  std::string line;
  while (std::getline(in, line)) {
    line = trim(line);
    if (!line.empty()) {
      lines.push_back(line);
    }
  }
  return lines;
}

std::string first_cpu_identifier() {
#ifdef _WIN32
  return run_command_trimmed(
      "wmic cpu get ProcessorId /value | findstr ProcessorId");
#else
  std::ifstream in("/proc/cpuinfo");
  std::string line;
  while (std::getline(in, line)) {
    auto pos = line.find(':');
    if (pos == std::string::npos) {
      continue;
    }
    std::string key = trim(line.substr(0, pos));
    if (key == "Serial" || key == "Hardware" || key == "model name" ||
        key == "Processor") {
      std::string value = trim(line.substr(pos + 1));
      if (!value.empty()) {
        return key + "=" + value;
      }
    }
  }
  return "";
#endif
}

std::vector<std::string> mac_addresses() {
  std::vector<std::string> macs;
  const std::regex mac_re(
      R"(^[0-9A-Fa-f]{2}(:[0-9A-Fa-f]{2}){5}$)");
#ifdef _WIN32
  std::string output = run_command_trimmed("getmac /fo csv /nh");
  std::istringstream lines(output);
  std::string line;
  while (std::getline(lines, line)) {
    auto first_quote = line.find('"');
    auto second_quote = line.find('"', first_quote + 1);
    if (first_quote != std::string::npos && second_quote != std::string::npos) {
      macs.push_back(line.substr(first_quote + 1, second_quote - first_quote - 1));
    }
  }
#else
  std::filesystem::path net_dir("/sys/class/net");
  if (std::filesystem::exists(net_dir)) {
    for (const auto& entry : std::filesystem::directory_iterator(net_dir)) {
      std::string name = entry.path().filename().string();
      if (name == "lo") {
        continue;
      }
      auto lines = read_lines(entry.path() / "address");
      if (!lines.empty()) {
        macs.push_back(lines.front());
      }
    }
  }
#endif
  macs.erase(std::remove_if(macs.begin(), macs.end(),
                            [&](const std::string& mac) {
                              return !std::regex_match(mac, mac_re) ||
                                     mac == "00:00:00:00:00:00";
                            }),
             macs.end());
  std::sort(macs.begin(), macs.end());
  macs.erase(std::unique(macs.begin(), macs.end()), macs.end());
  return macs;
}

std::string current_timestamp() {
  auto now = std::chrono::system_clock::now();
  std::time_t t = std::chrono::system_clock::to_time_t(now);
  std::tm tm{};
#ifdef _WIN32
  localtime_s(&tm, &t);
#else
  localtime_r(&t, &tm);
#endif
  std::ostringstream out;
  out << std::put_time(&tm, "%Y-%m-%dT%H:%M:%S%z");
  return out.str();
}

bool stdin_is_tty() {
#ifdef _WIN32
  return true;
#else
  return isatty(STDIN_FILENO) == 1;
#endif
}

}  // namespace

LicenseManager::LicenseManager(std::shared_ptr<spdlog::logger> logger)
    : logger_(std::move(logger)) {}

bool LicenseManager::IsM900Build() {
#ifdef M900BOX
  return true;
#else
  return false;
#endif
}

std::string LicenseManager::ComputeM900PlatformCiphertext(
    const std::string& baseboard_manufacturer,
    const std::string& baseboard_product_name,
    const std::string& baseboard_serial_number, const std::string& uid) {
  return sha256_hex(trim(baseboard_manufacturer) + "|" +
                    trim(baseboard_product_name) + "|" +
                    trim(baseboard_serial_number) + "|" + trim(uid) + "|" +
                    kLicenseSecret);
}

std::string LicenseManager::ExpectedAuthorizationCode(
    const std::string& machine_code) {
  return sha256_hex(uppercase(trim(machine_code)) + "|" + kLicenseSecret);
}

std::string LicenseManager::license_path() const {
  auto config_path =
      std::filesystem::path(RavBotConfig::DefaultConfigPath()).parent_path();
  return (config_path / "license.json").string();
}

std::string LicenseManager::records_path() const {
  auto config_path =
      std::filesystem::path(RavBotConfig::DefaultConfigPath()).parent_path();
  return (config_path / "license-records.json").string();
}

nlohmann::json LicenseManager::DeviceInfo() const {
  nlohmann::json info;
  info["cpu"] = first_cpu_identifier();
  info["macs"] = mac_addresses();
#ifndef _WIN32
  info["machineId"] = read_lines("/etc/machine-id").empty()
                          ? ""
                          : read_lines("/etc/machine-id").front();
#endif
  return info;
}

std::string LicenseManager::MachineCode() const {
  auto info = DeviceInfo();
  std::ostringstream raw;
  raw << info.value("cpu", "");
  if (info.contains("machineId")) {
    raw << "|" << info.value("machineId", "");
  }
  for (const auto& mac : info.value("macs", std::vector<std::string>{})) {
    raw << "|" << mac;
  }
  return sha256_hex(raw.str());
}

std::string LicenseManager::ActivationUrl() const {
  return std::string(kLicenseServer) + "/?machineCode=" + MachineCode();
}

LicenseStatus LicenseManager::Check() const {
  return IsM900Build() ? CheckM900() : CheckGeneric();
}

LicenseStatus LicenseManager::CheckM900() const {
  LicenseStatus status;
  status.m900_build = true;
  status.mode = "m900box";

#ifdef _WIN32
  status.message = "M900BOX validation is only implemented on Linux.";
  return status;
#else
  std::string manufacturer = run_command_trimmed("dmidecode -s baseboard-manufacturer");
  std::string product = run_command_trimmed("dmidecode -s baseboard-product-name");
  std::string serial = run_command_trimmed("dmidecode -s baseboard-serial-number");
  std::string uid = run_command_trimmed("dmidecode -t 11 | awk '/Flash ID:/ {print $5}'");
  std::string platform_info =
      uppercase(run_command_trimmed("dmidecode -t 11 | awk '/Platform Info:/ {print $5}'"));

  status.device_info = {{"baseboardManufacturer", manufacturer},
                        {"baseboardProductName", product},
                        {"baseboardSerialNumber", serial},
                        {"uid", uid},
                        {"platformInfo", platform_info}};

  if (manufacturer.empty() || product.empty() || serial.empty() ||
      uid.empty() || platform_info.empty()) {
    status.message =
        "M900BOX validation failed: missing dmidecode board information.";
    return status;
  }

  std::string expected =
      ComputeM900PlatformCiphertext(manufacturer, product, serial, uid);
  status.machine_code = expected;
  status.authorized = expected == platform_info;
  status.message = status.authorized
                       ? "M900BOX board validation passed."
                       : "M900BOX board validation failed.";
  return status;
#endif
}

LicenseStatus LicenseManager::CheckGeneric() const {
  LicenseStatus status;
  status.m900_build = false;
  status.mode = "generic";
  status.device_info = DeviceInfo();
  status.machine_code = MachineCode();

  std::ifstream in(license_path());
  if (!in) {
    status.message = "Generic RavBot is not activated.";
    return status;
  }

  nlohmann::json license;
  try {
    in >> license;
  } catch (...) {
    status.message = "Generic RavBot license file is invalid.";
    return status;
  }

  std::string stored_machine = uppercase(trim(license.value("machineCode", "")));
  std::string code = uppercase(trim(license.value("authorizationCode", "")));
  std::string expected = ExpectedAuthorizationCode(status.machine_code);
  status.authorized = stored_machine == status.machine_code && code == expected;
  status.message = status.authorized ? "Generic RavBot license is valid."
                                     : "Generic RavBot license is invalid.";
  return status;
}

void LicenseManager::SaveLicense(const std::string& authorization_code,
                                 const std::string& machine_code,
                                 const nlohmann::json& device_info) const {
  auto path = std::filesystem::path(license_path());
  std::filesystem::create_directories(path.parent_path());
  nlohmann::json license = {{"product", "RavBot"},
                            {"mode", "generic"},
                            {"machineCode", machine_code},
                            {"authorizationCode",
                             uppercase(trim(authorization_code))},
                            {"deviceInfo", device_info},
                            {"activatedAt", current_timestamp()}};
  std::ofstream out(path);
  out << license.dump(2) << "\n";
#ifndef _WIN32
  std::filesystem::permissions(
      path, std::filesystem::perms::owner_read |
                std::filesystem::perms::owner_write,
      std::filesystem::perm_options::replace);
#endif
}

void LicenseManager::AppendRecord(const std::string& authorization_code,
                                  const std::string& machine_code,
                                  const nlohmann::json& device_info) const {
  auto path = std::filesystem::path(records_path());
  std::filesystem::create_directories(path.parent_path());

  nlohmann::json records = nlohmann::json::array();
  {
    std::ifstream in(path);
    if (in) {
      try {
        in >> records;
      } catch (...) {
        records = nlohmann::json::array();
      }
    }
  }
  if (!records.is_array()) {
    records = nlohmann::json::array();
  }

  records.push_back({{"product", "RavBot"},
                     {"mode", "generic"},
                     {"machineCode", machine_code},
                     {"authorizationCode", uppercase(trim(authorization_code))},
                     {"deviceInfo", device_info},
                     {"activatedAt", current_timestamp()}});

  std::ofstream out(path);
  out << records.dump(2) << "\n";
#ifndef _WIN32
  std::filesystem::permissions(
      path, std::filesystem::perms::owner_read |
                std::filesystem::perms::owner_write,
      std::filesystem::perm_options::replace);
#endif
}

LicenseStatus LicenseManager::Activate(
    const std::string& authorization_code) const {
  LicenseStatus status;
  status.mode = "generic";
  status.device_info = DeviceInfo();
  status.machine_code = MachineCode();

  std::string code = uppercase(trim(authorization_code));
  if (code != ExpectedAuthorizationCode(status.machine_code)) {
    status.message = "Authorization code is invalid for this machine.";
    return status;
  }

  SaveLicense(code, status.machine_code, status.device_info);
  AppendRecord(code, status.machine_code, status.device_info);
  status.authorized = true;
  status.message = "Generic RavBot activation completed.";
  return status;
}

bool EnsureAuthorized(const std::shared_ptr<spdlog::logger>& logger,
                      bool allow_interactive_activation) {
  LicenseManager manager(logger);
  auto status = manager.Check();
  if (status.authorized) {
    return true;
  }

  if (logger) {
    logger->error("{}", status.message);
  }

  if (LicenseManager::IsM900Build()) {
    std::cerr << status.message << "\n";
    std::cerr << "This binary was built with M900BOX and can only run on a "
                 "validated M900BOX motherboard.\n";
    return false;
  }

  std::cerr << status.message << "\n";
  std::cerr << "Machine code: " << status.machine_code << "\n";
  std::cerr << "Activation URL: " << manager.ActivationUrl() << "\n";

  if (!allow_interactive_activation || !stdin_is_tty()) {
    std::cerr << "Run: ravbot license activate <authorization-code>\n";
    return false;
  }

  std::cerr << "Enter authorization code: ";
  std::string code;
  std::getline(std::cin, code);
  auto activated = manager.Activate(code);
  if (!activated.authorized) {
    std::cerr << activated.message << "\n";
    return false;
  }
  std::cerr << activated.message << "\n";
  return true;
}

int HandleLicenseCommand(const std::vector<std::string>& args,
                         const std::shared_ptr<spdlog::logger>& logger) {
  LicenseManager manager(logger);
  std::string sub = args.empty() ? "status" : args.front();

  if (sub == "machine-code") {
    std::cout << manager.MachineCode() << "\n";
    return 0;
  }

  if (sub == "activation-url") {
    std::cout << manager.ActivationUrl() << "\n";
    return 0;
  }

  if (sub == "activate") {
    if (LicenseManager::IsM900Build()) {
      std::cerr << "M900BOX builds do not use generic authorization codes.\n";
      return 1;
    }
    if (args.size() < 2) {
      std::cerr << "Usage: ravbot license activate <authorization-code>\n";
      return 1;
    }
    auto status = manager.Activate(args[1]);
    if (!status.authorized) {
      std::cerr << status.message << "\n";
      std::cerr << "Machine code: " << status.machine_code << "\n";
      return 1;
    }
    std::cout << status.message << "\n";
    return 0;
  }

  if (sub == "status") {
    auto status = manager.Check();
    nlohmann::json out = {{"authorized", status.authorized},
                          {"mode", status.mode},
                          {"m900Build", status.m900_build},
                          {"machineCode", status.machine_code},
                          {"activationUrl", manager.ActivationUrl()},
                          {"message", status.message},
                          {"deviceInfo", status.device_info}};
    std::cout << out.dump(2) << "\n";
    return status.authorized ? 0 : 1;
  }

  std::cerr << "Usage: ravbot license <status|machine-code|activation-url|activate>\n";
  return 1;
}

}  // namespace ravbot::licensing
