# Installation Guide

Detailed installation instructions for different platforms and use cases.

## System Requirements

### Minimum Requirements
- **CPU**: 2 cores, 2+ GHz
- **RAM**: 4 GB
- **Disk**: 2 GB free space
- **OS**: Linux (Ubuntu 20.04+), Windows (native or WSL2), or macOS 13+ on Apple Silicon

### Recommended Requirements
- **CPU**: 4+ cores
- **RAM**: 8+ GB
- **Disk**: 10 GB free space
- **GPU**: Optional (for faster inference if supported by your LLM provider)

## Linux Installation

### Ubuntu/Debian

#### Using Pre-built Binary
```bash
# Download the latest binary
wget https://github.com/RavBot/RavBot/releases/download/v1.0.0/ravbot-linux-x64.tar.gz

# Extract
tar xzf ravbot-linux-x64.tar.gz

# Install to system path
sudo mv ravbot /usr/local/bin/
chmod +x /usr/local/bin/ravbot

# Verify
ravbot --version
```

#### Build from Source
```bash
# Install build dependencies
sudo apt-get update
sudo apt-get install -y \
  build-essential \
  cmake \
  git \
  libssl-dev \
  nlohmann-json3-dev \
  libspdlog-dev

# Clone repository
git clone https://github.com/RavBot/RavBot.git
cd ravbot

# Build
mkdir build && cd build
cmake ..
cmake --build . --parallel

# Optional: Install system-wide
sudo cmake --install .

# Run tests
./ravbot_tests
```

#### Using Docker
```bash
# Pull image
docker pull ravbot:latest

# Run container
docker run -d \
  --name ravbot \
  -p 18800:18800 \
  -p 18801:18801 \
  -v ravbot_data:/home/ravbot/.ravbot \
  ravbot:latest

# View logs
docker logs ravbot
```

### Fedora/CentOS/RHEL

```bash
# Install dependencies
sudo dnf groupinstall "Development Tools" -y
sudo dnf install cmake openssl-devel nlohmann_json-devel spdlog-devel -y

# Build from source
git clone https://github.com/RavBot/RavBot.git
cd ravbot
mkdir build && cd build
cmake ..
cmake --build . --parallel
sudo cmake --install .
```

### Arch Linux

```bash
# Install from AUR (if available)
yay -S ravbot

# Or build from source
git clone https://github.com/RavBot/RavBot.git
cd ravbot
mkdir build && cd build
cmake ..
cmake --build . --parallel
sudo cmake --install .
```

## Windows Installation

### Windows 10/11 with WSL2

#### Setup WSL2
```powershell
# Enable WSL2
wsl --install

# Or if already installed
wsl --set-default-version 2
```

#### Install in WSL2
Follow the Ubuntu/Linux instructions above within WSL2:
```bash
wsl
cd ~
git clone https://github.com/RavBot/RavBot.git
# ... follow Linux build steps
```

#### Run from Windows
```powershell
# Forward WSL2 port to Windows
# In WSL2 terminal:
ravbot gateway

# In PowerShell (Windows):
# open http://localhost:18801
```

### Native Windows (MSVC)

#### Prerequisites
- **Visual Studio 2019+** or **Build Tools for Visual Studio**
- **CMake 3.15+**
- **Git**

#### Build Process
```batch
REM Clone repository
git clone https://github.com/RavBot/RavBot.git
cd ravbot

REM Create build directory
mkdir build
cd build

REM Generate Visual Studio project
cmake .. -G "Visual Studio 17 2022"

REM Build
cmake --build . --config Release -j %NUMBER_OF_PROCESSORS%

REM Tests
Release\ravbot_tests.exe
```

#### Install Dependencies (vcpkg)
```batch
REM If CMake can't find dependencies
vcpkg install nlohmann-json openssl spdlog zlib

REM Then configure with toolchain
cmake .. -DCMAKE_TOOLCHAIN_FILE=C:\path\to\vcpkg\scripts\buildsystems\vcpkg.cmake
```

#### Gateway Service Setup (Resolving Permission Issues)

Gateway service startup on Windows requires special handling:

**Option 1: Using Task Scheduler (Recommended)**

```powershell
# Run PowerShell as Administrator
cd path\to\RavBot
powershell -ExecutionPolicy Bypass -File scripts\gateway-setup-windows.ps1
```

This script will:
- ✅ Create a scheduled task (auto-start on boot)
- ✅ Generate startup helper scripts
- ✅ Create default configuration file
- ✅ Set up log directory

**Option 2: Manual Startup (No Admin Required)**

```batch
REM Double-click or run from command line
scripts\gateway-manual.bat

REM Or run directly
build\Release\ravbot.exe gateway run
```

**Option 3: WSL2 Alternative**

If you encounter Windows permission issues, WSL2 is recommended:

```powershell
# Install WSL2
wsl --install

# Use Linux installation in WSL2
wsl
cd ~/RavBot
# ... follow Linux build steps
```

#### Troubleshooting

**Q: `gateway install` command fails?**

A: On Windows, `gateway install` currently only creates a background process and does not automatically create a scheduled task. Please use Option 1 or Option 2 above.

**Q: Port is already in use?**

A: Modify the `gateway.port` value in the configuration file `~\.ravbot\ravbot.json`.

**Q: Antivirus blocking the application?**

A: Add `ravbot.exe` to your antivirus software's whitelist.

#### Windows Compatibility Notes

> - `NOMINMAX` is defined automatically to prevent conflicts between Windows API macros and C++ `std::min`/`std::max`.
> - `bcrypt` is linked automatically to satisfy Windows crypto/TLS library dependencies (for example, mbedtls).
> - `HTTPLIB_REQUIRE_ZLIB` is explicitly set to `OFF` when ZLIB is not present, preventing stale CMake cache issues on minimal environments.
> - The `logs -f` (follow) flag is not supported on Windows; use `logs -n <count>` to view recent entries.

### Docker on Windows
```powershell
# Pull and run Docker container
docker pull ravbot:latest

docker run -d `
  --name ravbot `
  -p 18800:18800 `
  -p 18801:18801 `
  -v ravbot_data:/home/ravbot/.ravbot `
  ravbot:latest

# View logs
docker logs ravbot

# Stop container
docker stop ravbot
```

## macOS Installation

### Recommended: install script

```bash
git clone https://github.com/RavBot/RavBot.git
cd RavBot
bash scripts/install.sh --user
```

This installs `ravbot` into `~/.ravbot/bin`, runs `onboard --quick`, and writes the launchd service definition to `~/Library/LaunchAgents/com.ravbot.gateway.plist`.

### Build from Source (manual)

#### Install Dependencies
```bash
# Homebrew packages used by the supported build script
brew install cmake ninja pkg-config git spdlog openssl@3 curl node
```

#### Build
```bash
git clone https://github.com/RavBot/RavBot.git
cd RavBot

# Recommended scripted build
./scripts/build.sh --tests

# Run tests
ctest --test-dir build --output-on-failure

# Optional: install the background service definition
./build/ravbot gateway install
```

If you prefer manual CMake configuration on macOS, pass the Homebrew prefixes explicitly:

```bash
brew install openssl@3 curl
cmake -B build -G Ninja \
  -DOPENSSL_ROOT_DIR="$(brew --prefix openssl@3)" \
  -DCURL_ROOT="$(brew --prefix curl)"
cmake --build build --parallel
```

## Post-Installation Setup

### Initialize Configuration

```bash
# Interactive setup (recommended)
ravbot onboard

# Quick setup (use defaults)
ravbot onboard --quick
```

During setup, RavBot creates the config file, workspace, and gateway auth token. Add your provider API keys afterwards by editing `~/.ravbot/ravbot.json`.

### Verify Installation

```bash
# Check version
ravbot --version

# Run tests
ravbot status

# Test basic functionality
ravbot run "Hello, what's your name?"
```

### Configure Environment (Optional)

```bash
# Set default agent
export RAVBOT_AGENT_ID=main

# Set configuration directory
export RAVBOT_CONFIG_DIR=~/.ravbot

# Set log level
export RAVBOT_LOG_LEVEL=debug

# Set gateway port
export RAVBOT_GATEWAY_PORT=18800
```

## Updating RavBot

### From Binary
```bash
# Download new version
wget https://github.com/RavBot/RavBot/releases/download/v1.1.0/ravbot-linux-x64.tar.gz

# Backup old binary
cp /usr/local/bin/ravbot /usr/local/bin/ravbot.backup

# Install new version
tar xzf ravbot-linux-x64.tar.gz
sudo mv ravbot /usr/local/bin/

# Verify
ravbot --version
```

### From Source
```bash
cd ravbot
git pull origin main
cd build
cmake --build . --parallel
sudo cmake --install .
```

### From Homebrew
```bash
brew update
brew upgrade ravbot
```

### Docker
```bash
docker pull ravbot:latest
docker stop ravbot
docker rm ravbot

# Re-run with new image
docker run -d \
  --name ravbot \
  -p 18800:18800 \
  -p 18801:18801 \
  -v ravbot_data:/home/ravbot/.ravbot \
  ravbot:latest
```

## Troubleshooting

### Build Failures

**CMake not found**
```bash
# Ubuntu
sudo apt-get install cmake

# macOS
brew install cmake

# Windows
choco install cmake
```

**Missing dependencies**
```bash
# Ubuntu
sudo apt-get install libssl-dev nlohmann-json3-dev libspdlog-dev

# macOS
brew install openssl nlohmann-json spdlog
```

**C++ compiler issues**
```bash
# Update compiler
sudo apt-get install build-essential  # Ubuntu/Debian
brew install gcc                       # macOS
```

### Runtime Issues

**Port already in use**
```bash
# Change gateway port
ravbot gateway --port 9000

# Or find and kill process using port 18800
lsof -i :18800
kill -9 <PID>
```

**Permission denied errors**
```bash
# Ensure ~/.ravbot is writable
chmod 700 ~/.ravbot
chmod 600 ~/.ravbot/*
```

**Configuration issues**
```bash
# Validate configuration
ravbot config validate

# View schema
ravbot config schema

# Reset to defaults
ravbot onboard --reset
```

## Uninstallation

### Binary Installation
```bash
# Remove binary
sudo rm /usr/local/bin/ravbot

# Remove configuration (optional)
rm -rf ~/.ravbot
```

### Docker
```bash
docker stop ravbot
docker rm ravbot
docker rmi ravbot:latest
```

### From Source
```bash
cd ravbot/build
sudo cmake --uninstall

# Or manually
sudo rm /usr/local/bin/ravbot
sudo rm -rf /usr/local/include/ravbot
```

---

**Next**: [Configure your installation](/guide/configuration) or [get started running agents](/guide/getting-started).
