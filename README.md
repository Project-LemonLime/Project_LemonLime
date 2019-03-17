# Project_LemonPt (不稳定版)

A tiny judging environment for OI contest based on Project_LemonPlus

---

## Windows 支持被咕了

## Ubuntu 14 及以下的系统可能存在兼容性问题

---

# 和 Lemon 的不同

- Subtask Skip
- 子任务依赖
- 支持高 DPI
- 新增手动保存比赛、打开比赛目录（在 `文件` 菜单栏中）
- 支持子文件夹
- 自定义最大重新评测次数（最多10次，默认2次）
- 单题评测
- 全新的交互式试题（使用更加通用的方式，目前只支持 C++）
  - 交互库路径：交互使用的头文件。
  - 交互库名称：选手引用头文件的名称。
  - 接口实现路径：实现接口的源文件（grader.cpp）。
- 移除了自定义测试
- 移除了多线程评测

# 和 LemonPlus 的不同

- 增加整理文件功能：使所有的选手的子文件夹内外都有答案文件。
- 增加 帮助 > 指南：一个大坑（有哪位英语好的大佬帮忙中译英一下啊？）
- 默认的栈空间设置为和内存限制相同。
- 界面的小修改。

# 其它更新

- 图标和启动横幅
- 评测详情窗口的各种评测结果有了不同的颜色。

---

# 构建

## Windows

咕咕咕

## Linux

### 依赖环境

```bash
sudo apt-get install g++ gcc qt5-default make
```

### 下载源代码，在源代码目录中依次执行

```bash
gcc watcher_unix.c -o watcher_unix -O2
gcc realjudge.c -o realjudge_linux -O2
qmake lemon.pro
make
```

获得可执行文件 ``lemon`` 。

---

# 附件

## 把 LemonPt 打包！

在 linux 下，我们可以把整个 LemonPt 打包成一个可执行文件，使其在任何地方都可以不须依赖地运行。

为了实现这个功能，我们准备了 ``patchelf`` 和 ``linuxdeployqt`` 。

### linuxdeployqt

把 ``linuxdeployqt`` 复制到你的 ``/usr/local/bin`` 下，并给它运行权限。

### patchelf

在终端下，按顺序执行：

```plain
./configure
make
sudo make install
```

然后去 ``/usr/local/bin`` 下看一看是否有一个 ``patchelf`` 出现了。

### 准备就绪...

在你的 LemonPt 目录下，执行：

```plain
linuxdeployqt lemon -appimage
```

这可能需要很长时间。
之后，你可以发现一个以 ``appimage`` 为后缀的文件，而它是可以直接运行的。
把这个文件放到任何地方都可以运行。
那么 LemonPt 就打包完成了。Congratulations!
