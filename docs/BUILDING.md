# Building W2PP

This document describes how to build the W2PP project using CMake.

## Prerequisites

### Required Tools

- **CMake** 3.15 or higher
- **Visual Studio 2022** (or 2019) with C++ workload
- **Windows SDK** (comes with Visual Studio)

### Installing Prerequisites

1. **Install Visual Studio 2022**
   - Download from: https://visualstudio.microsoft.com/downloads/
   - Install the "Desktop development with C++" workload

2. **Install CMake**
   - Download from: https://cmake.org/download/
   - Or install via Visual Studio Installer (CMake tools for Windows)

## Build Instructions

### Quick Build

From the project root directory:

```powershell
# Configure the project (Visual Studio 2022)
cmake -B build -S . -G "Visual Studio 17 2022"

# Build Debug configuration
cmake --build build --config Debug

# Build Release configuration
cmake --build build --config Release
```

### Build All Configurations

```powershell
# Configure
cmake -B build -S . -G "Visual Studio 17 2022"

# Build both Debug and Release
cmake --build build --config Debug
cmake --build build --config Release
```

### Build Specific Target

```powershell
# Build only DBSrv
cmake --build build --config Release --target DBSrv

# Build only TMSrv
cmake --build build --config Release --target TMSrv

# Build only ClientPatch_v7662
cmake --build build --config Release --target ClientPatch_v7662
```

## Build Outputs

After a successful build, the outputs are located in:

### Debug Configuration
- `build/Build/Debug/Server/DBSrv/run/DBSrv.exe`
- `build/Build/Debug/Server/TMSrv/run/TMSrv.exe`
- `build/Build/Client/ClientPatch_v7662_d.dll`

### Release Configuration
- `build/Build/Release/Server/DBSrv/run/DBSrv.exe`
- `build/Build/Release/Server/TMSrv/run/TMSrv.exe`
- `build/Build/Client/ClientPatch_v7662.dll`

## Project Structure

The CMake build system is organized as follows:

```
W2PP/
├── CMakeLists.txt                 # Root CMake configuration
├── Code/
│   ├── CMakeLists.txt            # (included via subdirectories)
│   ├── Basedef.cpp/.h            # Common source files
│   ├── CPSock.cpp/.h             # Common socket library
│   ├── DBSrv/
│   │   ├── CMakeLists.txt        # DBSrv-specific configuration
│   │   └── ...
│   ├── TMSrv/
│   │   ├── CMakeLists.txt        # TMSrv-specific configuration
│   │   └── ...
│   └── ClientPatch_v7662/
│       ├── CMakeLists.txt        # Client patch configuration
│       └── ...
└── build/                         # Build directory (generated)
```

## CMake Options

### Available Generators

- **Visual Studio 2022**: `-G "Visual Studio 17 2022"` (recommended)
- **Visual Studio 2019**: `-G "Visual Studio 16 2019"`
- **Ninja**: `-G Ninja` (requires Ninja to be installed)

### Build Types

CMake supports the following configurations:
- `Debug` - Unoptimized build with debug symbols
- `Release` - Optimized build without debug symbols
- `RelWithDebInfo` - Optimized build with debug symbols
- `MinSizeRel` - Size-optimized build

## Troubleshooting

### UTF-8 Encoding Warnings

You may see warnings like:
```
warning C4828: The file contains a character starting at offset 0x... that is invalid in the current source character set
```

These warnings are harmless and come from source files containing non-ASCII characters (Portuguese text). The build will still succeed.

### Missing Windows SDK

If you get errors about missing Windows SDK, ensure you have the Windows SDK installed via Visual Studio Installer.

### Clean Build

To perform a clean build:

```powershell
# Remove build directory
Remove-Item -Recurse -Force build

# Reconfigure and rebuild
cmake -B build -S . -G "Visual Studio 17 2022"
cmake --build build --config Release
```

## IDE Integration

### Visual Studio

1. Open the project folder in Visual Studio (File → Open → CMake...)
2. Visual Studio will automatically detect the `CMakeLists.txt`
3. Select your target configuration from the toolbar
4. Build using Ctrl+Shift+B or Build → Build All

### Visual Studio Code

1. Install the "CMake Tools" extension
2. Open the project folder
3. Select your kit (Visual Studio 2022)
4. Configure and build using the CMake Tools sidebar

## Advanced Usage

### Install Targets

```powershell
# Install to a specific directory (default: C:\Program Files\W2PP)
cmake --install build --config Release --prefix "C:/W2PP"
```

### Generate IDE Project Files

```powershell
# Generate Visual Studio solution file
cmake -B build -S . -G "Visual Studio 17 2022"

# Open the generated solution
start build/W2PP.sln
```

## Notes

- The project uses C++14 standard
- Static runtime linking is used (/MT for Release, /MTd for Debug)
- Windows subsystem is used for all executables (not console)
- The ClientPatch_v7662 is built as a DLL, not an executable
