# Project_LemonLime (不稳定版)

A tiny judging environment for OI contest based on Project_LemonPlus

![LemonLimeLogo](icon.png)

## ~~Lemon 绿了！~~

---

## Windows 支持被咕了

## Ubuntu 14 及以下的系统可能存在兼容性问题

---




# 和 Lemon 的不同

- 任意选手的任意题目评测
- Subtask Skip
- 子任务依赖
- 新增手动保存比赛、打开比赛目录（在 `文件` 菜单栏中）
- 支持选择子文件夹还是非子文件夹
- 自定义最大重新评测次数
- 全新的交互式试题（使用更加通用的方式，目前只支持 C++）
  - 交互库路径：交互使用的头文件。
  - 交互库名称：选手引用头文件的名称。
  - 接口实现路径：实现接口的源文件（grader.cpp）。
- 支持高 DPI
- 移除了自定义测试
- 移除了多线程评测

# 和 LemonPlus 的不同

- 选手名单上的成绩将会有背景颜色，随着分数变化而变化。导出的 html 文件也有颜色。配色方案来自 IOI。
- 移除了单题测试。这将成为历史
- 增加整理文件功能：使所有的选手的子文件夹内外都有答案文件。
- 增加 帮助 > 指南：一个大坑（有哪位英语好的大佬帮忙中译英一下啊？）
- 默认的栈空间设置为和内存限制相同。

# 微小的贡献

- 评测完后会通知
- 自带的实数比较模式判断了 nan 和 inf。来自某出题人的提醒
- 图标和启动横幅
- 评测详情窗口的各种评测结果有了不同的颜色。
- 如果你在某个点得分了，那么在测试时的窗口会显示获得的分数、使用的时间和空间。
- 测试的时候选手表格每次刷新会按照排名而不是名字排序。看起来像滚榜。
- 更友好的界面

---

# 构建

## Windows

等 iotang 退役（快了）再说吧

## Linux

### 依赖环境

```bash
sudo apt-get install g++ gcc qt5-default make
```

### 下载源代码，在源代码目录中依次执行

```bash
g++ watcher_unix.cpp -o watcher_unix -O2
g++ realjudge.cpp -o realjudge_linux -O2
qmake lemon.pro
make
```

获得可执行文件 ``lemon`` 。

