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

Clone repo:

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

Add to PATH:

```powershell
$env:Path += ";$PWD"
[Environment]::SetEnvironmentVariable("Path", [Environment]::GetEnvironmentVariable("Path", "User") + ";C:\path\to\vcpkg", "User")
```

Install deps:

```powershell
cd C:\path\to\NeoWPF
vcpkg install --triplet x64-windows-static-md
```

Use Visual Studio and install the C++ workload. Install CMake too if it is not already available.

```powershell
winget install --id Kitware.CMake -e
```

Configure:

```powershell
cmake -S . -B build -G "Visual Studio" -A x64 -DNEOWPF_RENDERER_BACKEND=D3D11
```

CMake will pick the matching Visual Studio generator for your installed version.

Open the generated solution in Visual Studio and build it in Release or Debug.

## Linux

Install Git:

```bash
sudo apt update
sudo apt install -y git
```

Clone repo:

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

Add to PATH:

```bash
echo 'export PATH="$PATH:$HOME/vcpkg"' >> ~/.bashrc
source ~/.bashrc
```

Install Linux deps:

```bash
sudo apt install -y build-essential cmake ninja-build pkg-config \
  libvulkan-dev glslang-tools wayland-protocols libwayland-dev libxkbcommon-dev \
  libfreetype6-dev libharfbuzz-dev libfontconfig1-dev zlib1g-dev libpng-dev
```

Install NeoWPF deps:

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

Open the generated project in Qt Creator or CLion and build it in Release.

## Required env var

Set `NEOWPF_WINDOWS_SDK_ROOT` when building with static dependencies on Windows:

```powershell
$env:NEOWPF_WINDOWS_SDK_ROOT = 'C:\Program Files (x86)\Windows Kits\10\Lib\10.0.26100.0\um\x64'
```

If it is missing, NeoWPF stops during configure.

## Info

- Windows: `x64-windows-static-md` + `D3D11`
- Linux: `x64-linux` + `Vulkan`

If `vcpkg` is missing or not on PATH, configure will fail.
