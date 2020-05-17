<img src="pics/icon.png" align=right />

# Project_LemonLime (Beta)

为了 OI 比赛而生的基于 Lemon + LemonPlus 的轻量评测系统

A tiny judging environment for OI contest based on Lemon + LemonPlus

**现已支持 Linux ![Ubuntu](https://github.com/iotang/Project_LemonLime/workflows/Ubuntu/badge.svg)，Windows ![Windows](https://github.com/iotang/Project_LemonLime/workflows/Windows/badge.svg)，以及 macOS ![MacOS](https://github.com/iotang/Project_LemonLime/workflows/MacOS/badge.svg)**

[![Codacy Badge](https://api.codacy.com/project/badge/Grade/9034d385758e485b8d8364b6e7c86f59)](https://app.codacy.com/manual/iotang/Project_LemonLime?utm_source=github.com&utm_medium=referral&utm_content=iotang/Project_LemonLime&utm_campaign=Badge_Grade_Dashboard)

已在这些系统测试：

|系统名称|版本号|位数|DE / WM|
|:--:|:--:|:--:|:--:|
|Windows|7|32, 64|Untitled|
|Windows|10|64|Untitled|
|Manjaro|20.0|64|KDE-Xorg; i3WM|
|Arch|2020-3-20|64|KDE-Xorg; KDE-Wayland|
|Ubuntu|20.04|64|GNOME 3|
|Ubuntu|18.04.4|64|GNOME 3|
|NOI Linux (Ubuntu) \*|14.04|32|GNOME 2|
|Linux Mint|19.3|64|Cinnamon|
|Deepin|15.11|64|DDE|
|Debian|10.3.0|64|LXQt; KDE-Xorg|
|Fedora|31-1.9|64|XFCE|
|openSUSE|Leap 15.1|64|iceWM|

## 特色

**Lemon 绿了！**

### 功能追加

-   题目类型支持：传统题，提交答案题，交互题（仅 C++），通信题。
-   统计：对比赛分数数据进行分析的栏目，还需要很多的更新。
-   整理文件：使所有的选手的子文件夹内外都有答案文件，并且删除大部分无用文件。支持在这之前备份文件。
-   增强测试点调整器：对测试点进行更迅捷的操作，比如移动、合并与拆分。在题目概要栏右键题目可以进入！（这个功能还在测试阶段，小心使用！）
-   选手名单上的成绩将会有背景颜色，随着分数变化而变化。导出的 HTML 文件也有颜色，配色方案大体来自 IOI，不过你也可以自定义（比如实现深色主题）。
-   支持重新排列题目顺序
-   窗口下方新增提示栏
-   移除了自定义测试
-   移除了多线程评测

### 评测改进

-   实数比较模式现在将同时比较绝对误差和相对误差，并且区分 `nan` 和 `inf`
-   任意选手的任意题目评测
-   可以控制的最大重新评测次数
-   Subtask Skip
-   子任务依赖
-   支持选择子文件夹还是非子文件夹
-   各种评测结果在评测时界面、结果查看界面和 HTML 有了易于区分的不同的颜色。
-   默认的栈空间设置为和内存限制相同。

### 用户体验

-   支持高 DPI
-   如果你在某个点得分了，那么在测试时的窗口会显示获得的分数、使用的时间和空间。
-   一键评测所有出现 找不到文件/编译问题 的记录
-   手动保存比赛、打开比赛目录（在 `文件` 菜单栏中）
-   重命名比赛
-   自动添加试题的时候每个点的分数不再是下取整(总分/数据点个数)。
-   减小了导出 HTM 的体积，并且给 HTML 添加了更多跳转。
-   默认空间限制和比较模式调整
-   图标和启动横幅
-   更友好的界面

## 构建

### 下载源码

#### 下载的东西太大了？

`git clone` 的时候，使用 `--depth 1` 可以使下载下来的文件大小减少很多（因为默认情况下它会把所有历史记录全部下载下来）。

#### 如果 Github 还是太慢…

你也许可以到 ``码云（Gitee）`` 去下载。

在很多地区，从 ``码云`` 下载的速度是从 ``Github`` 下载的速度的 100 倍。

[这个仓库在码云下的复制](https://gitee.com/iotang/Project_LemonLime)

### Windows

去 `Releases` 下载就可以了。

当然如果你装有 Qt 5，也可以下载源码编译。

#### 非常严重的提示

由于 Windows 的特殊性，请在下载 `Releases` 后检查 LemonLime 的功能的完整性，比如是否能探测程序的运行时间和使用内存。不过如果使用源码构建 LemonLime 的话将不会出现这种问题，所以仍然推荐使用源码构建 LemonLime。

> 在很多地方，下载 Qt 的时间 + 安装 Qt 的时间 + 下载 LemonLime 源代码的时间 + 编译的时间 < 从 Github 上下载可执行文件的时间。
>
> 下载 Qt 请考虑一个快速的国内镜像。

### Linux 

#### Arch Linux 系

已在这些系统测试：

|系统名称|版本号|位数|DE / WM|
|:--:|:--:|:--:|:--:|
|Arch|2020-3-20|64|KDE-Xorg; KDE-Wayland|
|Manjaro|20.0|64|KDE-Xorg; i3WM|

```bash
## 迅速安装 ##
yay -S lemon-lime # 版本可能过旧
yay -S lemon-lime-git 
# 感谢 @ayalhw 的支持。

## 使用 qmake ##
sudo pacman -S gcc make qt5-base # 依赖环境
cd 源代码的目录
g++ watcher_unix.cpp -o watcher_unix -O2
qmake lemon.pro
make # 获得可执行文件 lemon

## 使用 QtCreator ##
sudo pacman -S qtcreator
```

#### Debian | Ubuntu 系

已在这些系统测试：

|系统名称|版本号|位数|DE / WM|
|:--:|:--:|:--:|:--:|
|Ubuntu|20.04|64|GNOME 3|
|Ubuntu|18.04.4|64|GNOME 3|
|Linux Mint|19.3|64|Cinnamon|
|Deepin|15.11|64|DDE|
|Debian|10.3.0|64|LXQt; KDE-Xorg|

```bash
## 使用 qmake ##
sudo apt install qt5-default build-essential # 依赖环境
cd 源代码的目录
g++ watcher_unix.cpp -o watcher_unix -O2
qmake lemon.pro
make # 获得可执行文件 lemon

## 使用 QtCreator ##
sudo apt install qtcreator
```

##### Ubuntu 16 及更老

已在这些系统测试：

|系统名称|版本号|位数|DE / WM|
|:--:|:--:|:--:|:--:|
|Ubuntu|16.04|64|Unity|

Ubuntu 16 用 apt 安装的 Qt 版本只能到 5.5。

在 qmake 前你需要：
-   把代码中所有 `qAsConst`删除。

##### \* NOI Linux （Ubuntu 14 及更老）

已在这些系统测试：

|系统名称|版本号|位数|DE / WM|
|:--:|:--:|:--:|:--:|
|NOI Linux (Ubuntu) \*|14.04|32|GNOME 2|

NOI Linux 是 Ubuntu 14.04 的换皮，所以用 apt 安装的 Qt 版本只能到 5.2。

在 qmake 前你需要：
-   删除 `lemon.ui` 里面的 `<property name="tabBarAutoHide">...` 开始的 3 行。因为 Qt 5.2 里面还没有这个特性！
-   删除 `lemon.pro` 的 `unix:QMAKE_LFLAGS += -no-pie` 那一行。
-   把代码中所有 `asprintf` 换成 `sprintf`。

或者
-   在 Qt 官网上找一个更高版本（比如 5.12）的 Qt 安装。

*arbiter 退出了群聊。*

#### Fedora 系

已在这些系统测试：

|系统名称|版本号|位数|DE / WM|
|:--:|:--:|:--:|:--:|
|Fedora|31-1.9|64|XFCE|

```bash
## 使用 qmake ##
sudo yum install g++ make qt5 # 依赖环境
cd 源代码的目录
g++ watcher_unix.cpp -o watcher_unix -O2
qmake-qt5 lemon.pro
make # 获得可执行文件 lemon
```

#### openSUSE 系

已在这些系统测试：

|系统名称|版本号|位数|DE / WM|
|:--:|:--:|:--:|:--:|
|openSUSE|Leap 15.1|64|iceWM|

```bash
## 使用 QtCreator ##
sudo zypper install --type pattern devel_basis
sudo zypper install libqt5-creator
```

### macOS

在没有 macOS 机子的情况下写 macOS 支持是一件非常滑稽的事。

请使用 ``watcher_macos.cpp`` 编译 ``watcher_unix``，否则内存限制会出问题。

```bash
clang++ watcher_macos.cpp -o watcher_unix -O2
qmake lemon.pro
make
```
