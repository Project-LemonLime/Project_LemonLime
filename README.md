<img src="assets/icons/lemon-lime.png" align=right />

# Project LemonLime (Beta)

为了 OI 比赛而生的基于 Lemon + LemonPlus 的轻量评测系统

A tiny judging environment for OI contest based on Lemon + LemonPlus

同时支持 Qt5 和 Qt6, 使用 `-DLEMON_QT6=ON` 以使用 Qt6 编译。

现已支持 Linux ![Ubuntu](https://github.com/Project-LemonLime/Project_LemonLime/workflows/CPack%20-%20DEB%20-%20Ubuntu/badge.svg)
，Windows ![Windows](https://github.com/Project-LemonLime/Project_LemonLime/workflows/Windows/badge.svg)，以及 macOS ![MacOS](https://github.com/Project-LemonLime/Project_LemonLime/workflows/MacOS/badge.svg)

![CPack - DEB - Debian](https://github.com/Project-LemonLime/Project_LemonLime/workflows/CPack%20-%20DEB%20-%20Debian/badge.svg)
![CPack - DEB - Ubuntu](https://github.com/Project-LemonLime/Project_LemonLime/workflows/CPack%20-%20DEB%20-%20Ubuntu/badge.svg)

![Linux - AppImage](https://github.com/Project-LemonLime/Project_LemonLime/workflows/Lemon%20build%20matrix%20-%20AppImage/badge.svg)

[![FOSSA Status](https://app.fossa.com/api/projects/git%2Bgithub.com%2Fiotang%2FProject_LemonLime.svg?type=shield)](https://app.fossa.com/projects/git%2Bgithub.com%2Fiotang%2FProject_LemonLime?ref=badge_shield)

[![Codacy Badge](https://app.codacy.com/project/badge/Grade/ea760fd4674f44eaa6ca8273abc0669d)](https://www.codacy.com/gh/Project-LemonLime/Project_LemonLime/dashboard?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=Project-LemonLime/Project_LemonLime&amp;utm_campaign=Badge_Grade)

[![AUR version](https://img.shields.io/aur/version/lemon-lime?style=flat-square)](https://aur.archlinux.org/packages/lemon-lime/) ![AUR votes](https://img.shields.io/aur/votes/lemon-lime?label=lemon-lime%20vote&style=flat-square)

[![Packaging status](https://repology.org/badge/vertical-allrepos/lemon-lime.svg)](https://repology.org/metapackage/lemon-lime/versions)

曾在这些系统测试：

|          系统名称         |     版本号    |     架构     |     DE / WM    |
| :-------------------: | :--------: | :--------: | :------------: |
|        Windows        |      7     | x86, amd64 |    Untitled    |
|        Windows        |     10     |    amd64   |    Untitled    |
|        Manjaro        |   20.0.1   |    amd64   | KDE-Xorg; i3WM |
|          Arch         | 2020-10-31 |    amd64   |    KDE-Xorg    |
|          Arch         | 2020-10-31 |    amd64   |   KDE-Wayland  |
|         Ubuntu        |    20.04   |    amd64   |     GNOME 3    |
|         Ubuntu        |   18.04.4  |    amd64   |     GNOME 3    |
| NOI Linux (Ubuntu) \* |    14.04   |     x86    |     GNOME 2    |
|       Linux Mint      |    19.3    |    amd64   |    Cinnamon    |
|         Deepin        |    15.11   |    amd64   |       DDE      |
|         Deepin        |  20 (1000) |    amd64   |       DDE      |
|         Debian        |   10.3.0   |    amd64   | LXQt; KDE-Xorg |
|         Fedora        |   31-1.9   |    amd64   |      XFCE      |
|        openSUSE       |  Leap 15.1 |    amd64   |      iceWM     |
|        openSUSE       | Tumbleweed |    amd64   |    KDE-Xorg    |

如果您在您的系统上做了测试，请前往 [#49](https://github.com/Project-LemonLime/Project_LemonLime/issues/49) 告知

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

[Here](BUILD.md)

## Credit
```
Copyright (c) 2019-2021 Project LemonLime.

Libraries and other files that have been used in LemonLime are listed below:

Copyright (c) 2020 Itay Grudev (@itay-grudev): SingleApplication (MIT)

Copyright (c) 2020 Qv2ray Development Group (@Qv2ray): Design of Translator/Log, Project Structure and CI files (GPLv3)
```

## License

[![FOSSA Status](https://app.fossa.com/api/projects/git%2Bgithub.com%2Fiotang%2FProject_LemonLime.svg?type=large)](https://app.fossa.com/projects/git%2Bgithub.com%2Fiotang%2FProject_LemonLime?ref=badge_large)
