# NeoWPF build setup

## Windows

Install Git:

```powershell
winget install --id Git.Git -e
```

or:

```powershell
choco install git -y
```

Clone NeoWPF:

```powershell
git clone https://github.com/NeoWPF/NeoWPF.git
cd NeoWPF
```

Install vcpkg:

```powershell
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat
```

Add vcpkg to PATH:

```powershell
$env:Path += ";$PWD"
[Environment]::SetEnvironmentVariable("Path", [Environment]::GetEnvironmentVariable("Path", "User") + ";C:\path\to\vcpkg", "User")
```

Install the dependencies:

```powershell
cd C:\path\to\NeoWPF
vcpkg install --triplet x64-windows-static-md
```

Install Visual Studio with the C++ workload. CMake is also required:

```powershell
winget install --id Kitware.CMake -e
```

If using static dependencies, set the Windows SDK path:

```powershell
$env:NEOWPF_WINDOWS_SDK_ROOT = 'C:\Program Files (x86)\Windows Kits\10\Lib\10.0.26100.0\um\x64'
```

Configure:

```powershell
cmake -S . -B build -G "Visual Studio" -A x64 -DNEOWPF_RENDERER_BACKEND=D3D11
```

Open the generated solution in Visual Studio and build it in Debug or Release.

## Linux

Install the required packages:

```bash
sudo apt update
sudo apt install -y git build-essential cmake ninja-build pkg-config \
  libvulkan-dev glslang-tools wayland-protocols libwayland-dev libxkbcommon-dev \
  libfreetype6-dev libharfbuzz-dev libfontconfig1-dev zlib1g-dev libpng-dev
```

Clone NeoWPF:

```bash
git clone https://github.com/NeoWPF/NeoWPF.git
cd NeoWPF
```

Install vcpkg:

```bash
git clone https://github.com/microsoft/vcpkg.git ~/vcpkg
cd ~/vcpkg
./bootstrap-vcpkg.sh
```

Add vcpkg to PATH:

```bash
echo 'export PATH="$PATH:$HOME/vcpkg"' >> ~/.bashrc
source ~/.bashrc
```

Install the dependencies:

```bash
cd ~/NeoWPF
vcpkg install --triplet x64-linux
```

Configure:

```bash
cmake -S . -B build -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DNEOWPF_RENDERER_BACKEND=Vulkan \
  -DCMAKE_TOOLCHAIN_FILE="$HOME/vcpkg/scripts/buildsystems/vcpkg.cmake" \
  -DVCPKG_TARGET_TRIPLET=x64-linux
```

## Build defaults

Windows uses `x64-windows-static-md` with D3D11.

Linux uses `x64-linux` with Vulkan.

If vcpkg isn't installed or isn't available on PATH, CMake will fail to configure.
