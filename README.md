<img src="icon.png" align=right />

# Project_LemonLime (Beta)

A tiny judging environment for OI contest based on Project_LemonPlus

### 现已支持 Linux 和 Windows

并且…~~Lemon 绿了！~~



# 特色

## 功能追加

- 交互式试题支持（使用更加通用的方式，目前只支持 C++）
  - 交互库路径：交互使用的头文件。
  - 交互库名称：选手引用头文件的名称。
  - 接口实现路径：实现接口的源文件（grader.cpp）。
- 整理文件：使所有的选手的子文件夹内外都有答案文件，并且删除大部分无用文件。支持在这之前备份文件。
- 帮助 > 指南
- 重新排列题目顺序
- 窗口下方新增提示栏
- 移除了自定义测试
- 移除了多线程评测

## 评测改进

- 任意选手的任意题目评测
- 可以控制的最大重新评测次数
- Subtask Skip
- 子任务依赖
- 支持选择子文件夹还是非子文件夹
- 选手名单上的成绩将会有背景颜色，随着分数变化而变化。导出的 HTML 文件也有颜色。配色方案大体来自 IOI。
- 各种评测结果在评测时界面、结果查看界面和 HTML 也有了易于区分的不同的颜色。
- 默认的栈空间设置为和内存限制相同。

## 用户体验

- 支持高 DPI
- 自带的实数比较模式判断了 ``nan`` 和 ``inf``。来自某出题人的提醒
- 如果你在某个点得分了，那么在测试时的窗口会显示获得的分数、使用的时间和空间。
- 一键评测所有出现 找不到文件/编译问题 的记录
- 手动保存比赛、打开比赛目录（在 `文件` 菜单栏中）
- 重命名比赛
- 自动添加试题的时候每个点的分数不再是下取整(总分/数据点个数)。
- 减小了导出 HTM 的体积，并且给 HTML 添加了更多跳转。
- 默认空间限制和比较模式调整
- 图标和启动横幅
- 更友好的界面



# 构建

## Windows

去 `Releases` 下载可执行文件就可以了。

当然如果你装有 Qt 5，也可以下载源码编译。

## Linux 

### 依赖环境

```bash
sudo apt install build-essential qt5-default
```

### 下载源代码，在源代码目录中依次执行

```bash
g++ watcher_unix.cpp -o watcher_unix -O2
g++ realjudge.cpp -o realjudge_linux -O2
qmake lemon.pro
make
```

获得可执行文件 ``lemon`` 。

当然如果你装有 Qt Creator，也可以用它编译 release 版本。

```bash
sudo apt install qtcreator
```

## macOS（！？！？！？）

在没有 macOS 机子的情况下写 macOS 支持是一件非常滑稽的事。

请使用``watcher_macos.cpp``编译``watcher_unix``。

```bash
g++ watcher_macos.cpp -o watcher_unix -O2
```

