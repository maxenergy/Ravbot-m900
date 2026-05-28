// Copyright 2025 RavBot Contributors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <memory>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

namespace ravbot::licensing {

struct LicenseStatus {
  bool authorized = false;
  bool m900_build = false;
  std::string mode;
  std::string machine_code;
  std::string message;
  nlohmann::json device_info;
};

class LicenseManager {
 public:
  explicit LicenseManager(std::shared_ptr<spdlog::logger> logger = nullptr);

  LicenseStatus Check() const;
  LicenseStatus Activate(const std::string& authorization_code) const;
  std::string MachineCode() const;
  nlohmann::json DeviceInfo() const;
  std::string ActivationUrl() const;

  static bool IsM900Build();
  static std::string ExpectedAuthorizationCode(const std::string& machine_code);
  static std::string ComputeM900PlatformCiphertext(
      const std::string& baseboard_manufacturer,
      const std::string& baseboard_product_name,
      const std::string& baseboard_serial_number, const std::string& uid);

 private:
  std::shared_ptr<spdlog::logger> logger_;

  std::string license_path() const;
  std::string records_path() const;
  LicenseStatus CheckM900() const;
  LicenseStatus CheckGeneric() const;
  void SaveLicense(const std::string& authorization_code,
                   const std::string& machine_code,
                   const nlohmann::json& device_info) const;
  void AppendRecord(const std::string& authorization_code,
                    const std::string& machine_code,
                    const nlohmann::json& device_info) const;
};

bool EnsureAuthorized(const std::shared_ptr<spdlog::logger>& logger,
                      bool allow_interactive_activation = true);

int HandleLicenseCommand(const std::vector<std::string>& args,
                         const std::shared_ptr<spdlog::logger>& logger);

}  // namespace ravbot::licensing
