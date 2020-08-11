<img src="assets/icons/lemon-lime.png" align=right />

# Project LemonLime (Beta)

为了 OI 比赛而生的基于 Lemon + LemonPlus 的轻量评测系统

A tiny judging environment for OI contest based on Lemon + LemonPlus

现已支持 Linux ![Ubuntu](https://github.com/iotang/Project_LemonLime/workflows/CPack%20-%20DEB%20-%20Ubuntu/badge.svg)
，Windows ![Windows](https://github.com/iotang/Project_LemonLime/workflows/Windows/badge.svg)，以及 macOS ![MacOS](https://github.com/iotang/Project_LemonLime/workflows/MacOS/badge.svg)

![CPack - DEB - Debian](https://github.com/iotang/Project_LemonLime/workflows/CPack%20-%20DEB%20-%20Debian/badge.svg)
![CPack - DEB - Ubuntu](https://github.com/iotang/Project_LemonLime/workflows/CPack%20-%20DEB%20-%20Ubuntu/badge.svg)

![Linux - AppImage](https://github.com/iotang/Project_LemonLime/workflows/Lemon%20build%20matrix%20-%20AppImage/badge.svg)

[![FOSSA Status](https://app.fossa.com/api/projects/git%2Bgithub.com%2Fiotang%2FProject_LemonLime.svg?type=shield)](https://app.fossa.com/projects/git%2Bgithub.com%2Fiotang%2FProject_LemonLime?ref=badge_shield)

[![Codacy Badge](https://api.codacy.com/project/badge/Grade/9034d385758e485b8d8364b6e7c86f59)](https://app.codacy.com/manual/iotang/Project_LemonLime?utm_source=github.com&utm_medium=referral&utm_content=iotang/Project_LemonLime&utm_campaign=Badge_Grade_Dashboard)

[![AUR version](https://img.shields.io/aur/version/lemon-lime?style=flat-square)](https://aur.archlinux.org/packages/lemon-lime/) ![AUR votes](https://img.shields.io/aur/votes/lemon-lime?label=lemon-lime%20vote&style=flat-square)

[![Packaging status](https://repology.org/badge/vertical-allrepos/lemon-lime.svg)](https://repology.org/metapackage/lemon-lime/versions)

曾在这些系统测试：

|系统名称|版本号|位数|DE / WM|
|:--:|:--:|:--:|:--:|
|Windows|7|32, 64|Untitled|
|Windows|10|64|Untitled|
|Manjaro|20.0.1|64|KDE-Xorg; i3WM|
|Arch|2020-8-11|64|KDE-Xorg|
|Arch|2020-8-11|64|KDE-Wayland|
|Ubuntu|20.04|64|GNOME 3|
|Ubuntu|18.04.4|64|GNOME 3|
|NOI Linux (Ubuntu) \*|14.04|32|GNOME 2|
|Linux Mint|19.3|64|Cinnamon|
|Deepin|15.11|64|DDE|
|Debian|10.3.0|64|LXQt; KDE-Xorg|
|Fedora|31-1.9|64|XFCE|
|openSUSE|Leap 15.1|64|iceWM|

如果您在您的系统上做了测试，请前往 [#49](https://github.com/iotang/Project_LemonLime/issues/49) 告知

## 特色

以下是一些非常重要的改动：
-   **Lemon 绿了！**
-   LemonLime 现在支持 4 种题目类型：传统题、提交答案题、交互题，以及通信题。不过交互题和通信题暂时只确保 C++ 的支持。
-   现在在选手栏中，每个选手的每个题目都可以单独选择，而不是只能一行一行地选。现在你可以方便地重测某道题，还支持一键测试未测试 / 未找到源文件 / 编译错误等操作。
-   原先的 Lemon 的重测制度因为过于耗时而被废弃。现在，你可以自定义最大的重新评测次数。
-   自带的实数比较模式现在将同时比较绝对误差和相对误差，并且对 `nan` 和 `inf` 做出了判断。
-   在 Linux 下，默认的栈空间设置为和内存限制相同。
-   多线程评测和自定义测试被移除。

这里是一些值得称道的新功能：
-   Subtask Skip，某些时候的刚需。
-   现在你可以给每个测试点设置子任务依赖，而不是像以前把同一个数据加到一个又一个测试点中。
-   在题目概要栏右键题目，可以进入增强测试点调整器。CCR 的出色功能，现在 LemonLime 也支持了。（这个功能还在测试阶段，小心使用！）
-   增加统计栏目，对比赛分数数据进行简要的分析。还需要很多的更新。
-   新增的整理文件功能，可以使所有的选手的子文件夹内外都有答案文件，并且删除大部分无用文件。支持在这之前备份文件。
-   你也可以对每一个题目，选择是在子文件夹内寻找源文件还是子文件夹外。

还有一些令人舒适的小变化：
-   支持高 DPI。
-   选手名单上的成绩将会有背景颜色，随着分数变化而变化。导出的 HTML 文件也有颜色，默认配色方案大体来自 IOI，不过你也可以自定义（比如实现深色主题）。
-   各种评测结果在评测时界面、结果查看界面和导出的 HTML 文件有了易于区分的不同的颜色。
-   支持重新排列题目顺序。在有些时候它很重要。
-   支持重命名比赛。
-   手动保存比赛、打开比赛目录（在 `文件` 菜单栏中）。
-   窗口下方新增提示栏。

以及，一点小细节：
-   如果你在某个点得分了，那么在测试时的窗口会显示获得的分数、使用的时间和空间。
-   逐行比较模式现在可以显示出错位置的行号了。
-   减小了导出 HTM 的体积，并且给 HTML 添加了更多跳转。
-   自动添加试题的时候每个点的分数不再是下取整 (总分 / 数据点个数)。
-   改动了图标和启动横幅。
-   无处不在的界面优化。

## 构建

### 源码下载

```plain
$ git clone https://github.com/iotang/Project_LemonLime.git --recursive
```

#### 下载的东西太大了？

`git clone` 的时候，使用 `--depth=1` 可以使下载下来的文件大小减少很多（因为默认情况下它会把所有历史记录全部下载下来）。

#### 如果 Github 还是太慢…

你也许可以到 ``码云（Gitee）`` 去下载。

在很多地区，从 ``码云`` 下载的速度是从 ``Github`` 下载的速度的 100 倍。

[这个仓库在码云下的复制](https://gitee.com/iotang/Project_LemonLime)

### Windows

去 `Releases` 下载就可以了。

#### Scoop 用户

添加[第三方 bucket](https://github.com/ChungZH/peach) 即可快速安装与更新

```powershell
scoop bucket add peach https://github.com/ChungZH/peach
scoop install peach/lemon
```

当然如果你装有 Qt 5，也可以下载源码编译。

#### 非常严重的提示

由于 Windows 的特殊性，请在下载 `Releases` 后检查 LemonLime 的功能的完整性，比如是否能探测程序的运行时间和使用内存。不过如果使用源码构建 LemonLime 的话将不会出现这种问题，所以仍然推荐使用源码构建 LemonLime。

> 在很多地方，下载 Qt 的时间 + 安装 Qt 的时间 + 下载 LemonLime 源代码的时间 + 编译的时间 < 从 Github 上下载可执行文件的时间。
>
> 下载 Qt 请考虑一个快速的国内镜像。

### Linux 

#### Arch Linux 系

```bash
## 迅速安装 ##
yay -S lemon-lime # 稳定版本
yay -S lemon-lime-git # 开发版本（提前使用许多新功能！）
# 感谢 @ayalhw 的支持。

## 使用 CMake ##
sudo pacman -S gcc cmake qt5-base ninja make # 依赖环境(ninja 和 make 二选一)
cd 源代码的目录
g++ watcher_unix.cpp -o watcher_unix -O2
cmake . -DCMAKE_BUILD_TYPE=Release -GNinja # 如使用 make 请删去 -GNinja
cmake --build .  # 获得可执行文件 lemon

## 使用 QtCreator ##
sudo pacman -S qtcreator
```

#### Debian | Ubuntu 系

```bash
## 使用 qmake ##
sudo apt install qt5-default build-essential ninja-build qtbase5-dev qttools5-dev cmake # 依赖环境, ninja 可选
cd 源代码的目录
g++ watcher_unix.cpp -o watcher_unix -O2
cmake . -DCMAKE_BUILD_TYPE=Release -GNinja # 如使用 make 请删去 -GNinja
cmake --build .  # 获得可执行文件 lemon

## 使用 QtCreator ##
sudo apt install qtcreator
```

##### \* Ubuntu 18 及更老 (包括 NOI Linux 这种毒瘤

Ubuntu 18 用 apt 安装的 Qt 版本只能到 5.9。

Ubuntu 16 用 apt 安装的 Qt 版本只能到 5.5。
GCC 不支持 C++17, 可以自己去 CMakeLists.txt 把标准改成 11, 能不能编译听天由命（目前还是可以的，等到迁移 Qt6 估计就不行了

NOI Linux 是 Ubuntu 14.04 的换皮，所以用 apt 安装的 Qt 版本只能到 5.2。

*arbiter 退出了群聊。*

#### Fedora 系

```bash
## 使用 qmake ##
sudo yum install g++ make qt5 cmake ninja # 依赖环境(ninja 和 make 二选一)
cd 源代码的目录
g++ watcher_unix.cpp -o watcher_unix -O2
cmake . -DCMAKE_BUILD_TYPE=Release -GNinja # 如使用 make 请删去 -GNinja
cmake --build . # 获得可执行文件 lemon
```

#### openSUSE 系

```bash
## 使用 QtCreator ##
sudo zypper install --type pattern devel_basis
sudo zypper install libqt5-creator
```

### etc.

有 AppImage 可用（虽然丑了点

前往 Actions 或者 Release 界面下载，
```chmod +x xxxx.AppImage``` 再执行即可（注意从 Actions 下载到的是一个压缩包

PS：有可能会提示缺少 fuse, 请安装

### macOS

在没有 macOS 机子的情况下写 macOS 支持是一件非常滑稽的事。

请使用 ``watcher_macos.cpp`` 编译 ``watcher_unix``，否则内存限制会出问题。

```bash
clang++ watcher_macos.cpp -o watcher_unix -O2
cmake .
cmake --build .
```

## 致谢

-   [SingleApplication](https://github.com/itay-grudev/SingleApplication): 是 Qt5 中 QtSingleApplication 的支持实例通信的增强版。


## License
[![FOSSA Status](https://app.fossa.com/api/projects/git%2Bgithub.com%2Fiotang%2FProject_LemonLime.svg?type=large)](https://app.fossa.com/projects/git%2Bgithub.com%2Fiotang%2FProject_LemonLime?ref=badge_large)
