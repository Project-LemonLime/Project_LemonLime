# 从源码构建 LemonLime

## 源码下载

```plain
$ git clone https://github.com/Project-LemonLime/Project_LemonLime.git --recursive
```

### 下载的东西太大了？

`git clone` 的时候，使用 `--depth=1 --shallow-submodules` 可以使下载下来的文件大小减少很多（因为默认情况下它会把所有历史记录全部下载下来）。GitHub 自动生成的源码压缩包缺少子模块内容，应优先使用 `git clone`。

### 如果 Github 还是太慢…

你也许可以到 `码云（Gitee）` 去下载。

在很多地区，从 `码云` 下载的速度是从 `Github` 下载的速度的 100 倍。

[这个仓库在码云下的镜像](https://gitee.com/iotang/Project_LemonLime)

## 构建依赖

| 依赖           | 要求                                                  |
| -------------- | ----------------------------------------------------- |
| CMake          | 推荐 3.20 或更高；                                    |
| C++ 编译器     | 支持 C++17，使用与 Qt 工具包匹配的编译器              |
| Qt             | 6.8 或更高                                            |
| Qt 模块        | Core、Gui、Widgets、Network、Svg、LinguistTools、Test |
| Linux 附加模块 | Qt DBus，用于评测期间向系统申请阻止休眠               |
| 构建工具       | Ninja，或 CMake 支持的其他生成器                      |
| Git            | 用于获取源代码、子模块及构建版本号                    |

对于现有仓库，在切换分支或更新代码后同步子模块：

```bash
git submodule update --init --recursive
```

## Windows

安装 Visual Studio 的“使用 C++ 的桌面开发”组件、Windows SDK、CMake、Ninja，以及相应架构的 Qt MSVC 工具包。在 Visual Studio 的 **Developer PowerShell** 中执行：

```powershell
cmake -S . -B build -G Ninja `
    -DCMAKE_BUILD_TYPE=Release `
    -DCMAKE_PREFIX_PATH="<your qt install path>/msvc2022_64"
cmake --build build --parallel
```

build 目录下会生成 lemon.exe。动态链接 Qt 时，使用 `windeployqt` 复制运行时依赖：

```powershell
& "<your qt install path>/msvc2022_64/bin/windeployqt.exe" --release build/lemon.exe
& ./build/lemon.exe
```

## Linux

安装发行版提供的 C++ 工具链、CMake、Ninja 和 Qt 开发包。

```bash
# Arch Linux
sudo pacman -S --needed base-devel cmake ninja qt6-base qt6-tools qt6-svg

# Debian 或 Ubuntu
sudo apt install build-essential cmake ninja-build pkg-config lsb-release \
    qt6-base-dev qt6-tools-dev qt6-tools-dev-tools qt6-l10n-tools qt6-svg-dev

# Fedora
sudo dnf install cmake qt6-qtbase-devel qt6-qttools-devel qt6-qttools-linguist qt6-qtsvg-devel desktop-file-utils ninja-build
cd 源代码的目录
cmake . -DCMAKE_BUILD_TYPE=Release -GNinja
ninja # 获得可执行文件 lemon

ninja --install # 将其安装到系统中，默认安装位置位于 /usr/local

# 或者直接生成 RPM 包
sudo dnf install cmake qt6-qtbase-devel qt6-linguist qt6-qtsvg-devel desktop-file-utils ninja-build redhat-lsb-core fedora-packager rpmdevtools
cmake . -DCMAKE_BUILD_TYPE=Release -GNinja -DBUILD_RPM=ON
ninja

# openSUSE
sudo zypper in qt6-base-devel qt6-tools qt6-svg-devel ninja qt6-linguist-devel  # Qt6 依赖环境
cd 源代码的目录
cmake . -DCMAKE_BUILD_TYPE=Release -GNinja # 如使用 make 请删去 -GNinja
ninja # 获得可执行文件 lemon

ninja --install # 将其安装到系统中，默认安装位置位于 /usr/local

# 或者直接生成 RPM 包
sudo zypper in lsb-release rpm-build # RPM 依赖
cmake . -DCMAKE_BUILD_TYPE=Release -GNinja -DBUILD_RPM=ON
ninja

```

## macOS

在没有 macOS 机子的情况下写 macOS 支持是一件非常滑稽的事。

安装 Xcode Command Line Tools 和 Homebrew，然后安装构建依赖：

```bash
xcode-select --install
brew install cmake ninja qt
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_PREFIX_PATH="$(brew --prefix qt)"
cmake --build build --parallel
open build/lemon.app
```
