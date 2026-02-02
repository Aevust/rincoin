# Building Rincoin Release Binaries

This guide explains how to build official release binaries for Rincoin using the automated build script.

## Overview

The `contrib/build_release.sh` script automates the process of building release binaries for multiple platforms:
- **Linux** (x86_64) - Compatible with Ubuntu 20.04+, Debian 11+, RHEL 8+
- **Windows** (x64) - Compatible with Windows 10 and Windows 11

The script uses Docker to create reproducible builds in isolated environments and supports build caching for faster subsequent builds.

## Features

- ✅ Git-based builds from version tags
- ✅ Automatic source code packaging (tar.gz and zip)
- ✅ Cross-platform compilation (Linux and Windows)
- ✅ Build artifact caching for faster rebuilds
- ✅ SHA256 checksum generation
- ✅ Comprehensive release documentation
- ✅ Static C++ library linking for maximum compatibility

## Prerequisites

### System Requirements

- Linux host system (Debian 11+, Ubuntu 20.04+)
- At least 20GB free disk space
- 8GB+ RAM recommended
- Multi-core CPU (4+ cores recommended)

### Required Software

The build script requires the following tools:

- **Docker** - For isolated build environments
- **Git** - For repository management
- **tar** - For creating archives
- **gzip** - For compression
- **zip** - For Windows archives
- **Berkeley DB 4.8** - Must be pre-installed locally

### Installing Dependencies

#### Debian 13 (Trixie)

```bash
# Install Docker
sudo apt update
sudo apt install docker.io docker-compose
sudo systemctl enable --now docker
sudo usermod -aG docker $USER
# Log out and back in for group changes to take effect

# Install build tools
sudo apt install git tar gzip zip

# Install Berkeley DB 4.8 (required for wallet support)
cd /path/to/rincoin
./contrib/install_db4.sh $(pwd)
```

#### Ubuntu 24.04 (Noble Numbat)

```bash
# Install Docker
sudo apt update
sudo apt install docker.io docker-compose-v2
sudo systemctl enable --now docker
sudo usermod -aG docker $USER
# Log out and back in for group changes to take effect

# Install build tools
sudo apt install git tar gzip zip

# Install Berkeley DB 4.8 (required for wallet support)
cd /path/to/rincoin
./contrib/install_db4.sh $(pwd)
```

#### Verify Docker Installation

After logging out and back in, verify Docker works without sudo:

```bash
docker --version
docker ps
```

If you get permission errors, ensure you're in the docker group:

```bash
groups | grep docker
```

## Usage

### Basic Usage

Build release binaries from a git tag:

```bash
cd /path/to/rincoin
./contrib/build_release.sh <git-tag>
```

**Example:**
```bash
./contrib/build_release.sh v1.0.1
```

### Advanced Usage

#### With Custom Git Repository

```bash
./contrib/build_release.sh v1.0.1 https://github.com/YourFork/rincoin.git
```

#### Clean Build (Force Rebuild)

Remove all cached artifacts and rebuild from scratch:

```bash
./contrib/build_release.sh v1.0.1 --clean
```

This will:
- Remove Docker images
- Clear dependency caches
- Start fresh build

#### Combined Options

```bash
./contrib/build_release.sh v1.0.2 https://github.com/Rin-coin/rincoin.git --clean
```

## Build Process

The script performs the following steps:

### 1. Preparation
- Validates prerequisites (Docker, git, db4)
- Clones repository at specified tag
- Creates source packages (tar.gz and zip)

### 2. Linux Build (Ubuntu 20.04 base)
- Creates/reuses Docker image with build tools
- Compiles binaries with static libstdc++
- Generates: `rincoind`, `rincoin-cli`, `rincoin-tx`, `rincoin-wallet`, `rincoin-qt`
- Creates distribution tarball

### 3. Windows Build (MinGW cross-compilation)
- Creates/reuses Docker image with MinGW toolchain
- Builds Windows dependencies via depends system
- Compiles Windows binaries
- Generates: `*.exe` files
- Creates distribution zip

### 4. Finalization
- Generates SHA256 checksums for all artifacts
- Creates release documentation (README.txt)
- Organizes output in release-builds directory

## Build Caching

The script implements intelligent caching to speed up subsequent builds:

### Cached Artifacts

1. **Docker Images** (`~2-5 minutes saved`)
   - `rincoin-builder:linux`
   - `rincoin-builder:windows`

2. **Dependency Downloads** (`~5-10 minutes saved`)
   - Location: `.build-cache/depends-sources/`
   - Cached: boost, qt, libevent, openssl, etc.

3. **Built Dependencies** (`~20-40 minutes saved`)
   - Location: `.build-cache/depends-built/`
   - Cached: Compiled Windows dependencies

### Build Times

| Build Type | First Build | Cached Build | Savings |
|------------|-------------|--------------|---------|
| Linux only | ~15 min | ~8 min | ~50% |
| Windows only | ~60 min | ~15 min | ~75% |
| Both platforms | ~75 min | ~20 min | ~73% |

### Managing Cache

**View cache size:**
```bash
du -sh .build-cache/
```

**Clear cache manually:**
```bash
rm -rf .build-cache/
```

**Use --clean flag:**
```bash
./contrib/build_release.sh v1.0.1 --clean
```

## Output Structure

After a successful build, you'll find the following structure:

```
release-builds/
└── 1.0.1/
    ├── source/
    │   ├── rincoin-1.0.1.tar.gz        # Source code (tar.gz)
    │   ├── rincoin-1.0.1.zip            # Source code (zip)
    │   └── SHA256SUMS.txt                # Source checksums
    ├── binaries/
    │   ├── linux/
    │   │   ├── rincoind                  # Linux daemon
    │   │   ├── rincoin-cli               # Linux RPC client
    │   │   ├── rincoin-tx                # Linux transaction tool
    │   │   ├── rincoin-wallet            # Linux wallet tool
    │   │   ├── rincoin-qt                # Linux GUI
    │   │   └── SHA256SUMS.txt            # Binary checksums
    │   └── windows/
    │       ├── rincoind.exe              # Windows daemon
    │       ├── rincoin-cli.exe           # Windows RPC client
    │       ├── rincoin-tx.exe            # Windows transaction tool
    │       ├── rincoin-wallet.exe        # Windows wallet tool
    │       ├── rincoin-qt.exe            # Windows GUI
    │       └── SHA256SUMS.txt            # Binary checksums
    ├── tarballs/
    │   ├── rincoin-1.0.1-x86_64-linux-gnu.tar.gz   # Linux distribution
    │   ├── rincoin-1.0.1-win64.zip                  # Windows distribution
    │   └── SHA256SUMS.txt                            # Archive checksums
    └── README.txt                                    # Release documentation
```

## Verification

### Verify Checksums

**For source packages:**
```bash
cd release-builds/1.0.1/source/
sha256sum -c SHA256SUMS.txt
```

**For distribution archives:**
```bash
cd release-builds/1.0.1/tarballs/
sha256sum -c SHA256SUMS.txt
```

**For individual binaries:**
```bash
cd release-builds/1.0.1/binaries/linux/
sha256sum -c SHA256SUMS.txt
```

### Test Binaries

**Linux:**
```bash
./release-builds/1.0.1/binaries/linux/rincoind --version
./release-builds/1.0.1/binaries/linux/rincoin-cli --help
```

**Windows (on Windows system):**
```cmd
release-builds\1.0.1\binaries\windows\rincoind.exe --version
```

## Distribution

### For Linux Users

Provide the tarball:
```
rincoin-1.0.1-x86_64-linux-gnu.tar.gz
```

Installation:
```bash
tar xzf rincoin-1.0.1-x86_64-linux-gnu.tar.gz
sudo cp rincoin-1.0.1/bin/* /usr/local/bin/
```

### For Windows Users

Provide the zip archive:
```
rincoin-1.0.1-win64.zip
```

Installation:
- Extract the zip file
- Run `rincoin-qt.exe` for GUI
- Or run `rincoind.exe` for daemon mode

### For Developers

Provide the source packages:
```
rincoin-1.0.1.tar.gz
rincoin-1.0.1.zip
```

Build from source:
```bash
tar xzf rincoin-1.0.1.tar.gz
cd rincoin-1.0.1
./autogen.sh
./configure
make -j$(nproc)
sudo make install
```

## Troubleshooting

### Docker Permission Denied

**Problem:** `permission denied while trying to connect to the Docker daemon socket`

**Solution:**
```bash
sudo usermod -aG docker $USER
# Log out and back in
```

### Berkeley DB 4.8 Not Found

**Problem:** `Berkeley DB 4.8 not found at /path/to/db4`

**Solution:**
```bash
cd /path/to/rincoin
./contrib/install_db4.sh $(pwd)
```

### Build Fails with "No space left on device"

**Problem:** Not enough disk space

**Solution:**
```bash
# Check disk space
df -h

# Clear Docker cache
docker system prune -a

# Clear build cache
rm -rf .build-cache/
```

### Windows Build Takes Too Long

**Problem:** Windows build is very slow on first run

**Solution:** This is normal. The depends system builds all dependencies from source (Qt, Boost, etc.). Subsequent builds will be much faster thanks to caching. Use `--clean` flag only when necessary.

### Git Tag Not Found

**Problem:** `Failed to clone repository or checkout tag`

**Solution:**
```bash
# List available tags
cd /path/to/rincoin
git tag

# Ensure tag exists
git tag | grep v1.0.1

# Try with exact tag name
./contrib/build_release.sh v1.0.1
```

## Advanced Topics

### Building for Additional Architectures

The script currently supports x86_64 (Linux) and x64 (Windows). To add ARM support:

1. Modify `CROSS_HOSTS` array in the script
2. Add ARM cross-compilation toolchain to Dockerfile
3. Configure depends for ARM target

Example architectures from Bitcoin Core:
- `aarch64-linux-gnu` (64-bit ARM)
- `arm-linux-gnueabihf` (32-bit ARM)
- `riscv64-linux-gnu` (RISC-V 64-bit)

### Reproducible Builds

For deterministic/reproducible builds, consider using Gitian or Guix:

- **Gitian:** See `contrib/gitian-descriptors/`
- **Guix:** See `contrib/guix/` (if implemented)

These provide additional guarantees that the same source produces identical binaries.

### Continuous Integration

To integrate with CI/CD:

```yaml
# Example GitHub Actions workflow
name: Build Release
on:
  push:
    tags:
      - 'v*'
jobs:
  build:
    runs-on: ubuntu-22.04
    steps:
      - uses: actions/checkout@v3
      - name: Install dependencies
        run: |
          sudo apt-get update
          sudo apt-get install -y docker.io git tar gzip zip
          ./contrib/install_db4.sh $(pwd)
      - name: Build release
        run: ./contrib/build_release.sh ${{ github.ref_name }}
      - name: Upload artifacts
        uses: actions/upload-artifact@v3
        with:
          name: rincoin-${{ github.ref_name }}
          path: release-builds/
```

## Platform Compatibility

### Linux Binaries

Built on Ubuntu 20.04 (glibc 2.31), compatible with:

| Distribution | Minimum Version | Status |
|--------------|----------------|--------|
| Ubuntu | 20.04 LTS | ✅ Tested |
| Ubuntu | 22.04 LTS | ✅ Tested |
| Ubuntu | 24.04 LTS | ✅ Tested |
| Debian | 11 (Bullseye) | ✅ Compatible |
| Debian | 12 (Bookworm) | ✅ Compatible |
| Debian | 13 (Trixie) | ✅ Compatible |
| RHEL/Rocky/Alma | 8+ | ✅ Compatible |
| Fedora | 35+ | ✅ Compatible |
| Arch Linux | Current | ✅ Compatible |

**Not Supported:**
- CentOS 7 (glibc 2.17, EOL)
- Ubuntu 18.04 (glibc 2.27, EOL)
- Debian 10 (glibc 2.28, EOL)

### Windows Binaries

Built with MinGW, compatible with:

| Windows Version | Architecture | Status |
|-----------------|--------------|--------|
| Windows 10 | x64 | ✅ Supported |
| Windows 11 | x64 | ✅ Supported |
| Windows Server 2016+ | x64 | ✅ Supported |

**Not Supported:**
- Windows 7 (EOL)
- Windows 8/8.1 (EOL)
- 32-bit Windows

## Security Considerations

### Build Environment

- Builds run in isolated Docker containers
- No network access during compilation (only for dependency downloads)
- Uses official Ubuntu base images
- Source code cloned from git ensures authenticity

### Binary Verification

Always verify checksums:

```bash
# After download
sha256sum rincoin-1.0.1-x86_64-linux-gnu.tar.gz
# Compare with published SHA256SUMS.txt
```

### GPG Signing

For official releases, consider signing the binaries:

```bash
# Sign the checksums file
gpg --detach-sign --armor SHA256SUMS.txt

# Verify signature
gpg --verify SHA256SUMS.txt.asc SHA256SUMS.txt
```

## Additional Resources

- [Build Unix](build-unix.md) - Detailed Unix build instructions
- [Build Windows](build-windows.md) - Windows build documentation
- [Dependencies](dependencies.md) - Dependency version information
- [Gitian Building](gitian-building.md) - Deterministic build system
- [Release Process](release-process.md) - Official release procedures

## Support

For build-related issues:

- GitHub Issues: https://github.com/Rin-coin/rincoin/issues
- Discord: https://discord.gg/Ap7TUXYRBf
- Documentation: https://www.rincoin.net/

## License

This build script and documentation are released under the MIT License, same as Rincoin Core.
