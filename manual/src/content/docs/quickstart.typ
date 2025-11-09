#import "../html.typ" : *
#show : html-style
#metadata(( title: "快速开始" ))<frontmatter>

= 快速开始

LemonLime 是为了 OI 比赛而生的基于 Lemon + LemonPlus 的轻量评测系统，具有三大桌面系统支持。

可以在 #link("https://github.com/Project-LemonLime/Project_LemonLime/releases/")[Github] 上下载最新版本。若因网络问题下载速度较慢，可以自行寻找镜像。

在 Windows 系统下可以解压压缩包，双击 `lemon.exe` 运行程序。如无法打开，可以尝试安装同目录下的 `vc_redist.x64.exe` 解决。

在 Linux 系统下可以在软件目录下通过终端运行 `chmod +x ./LemonLime-linux-...` 赋予权限，然后直接运行 `./LemonLime-linux-...` 启动。

如果启动时出现：

```bash
error while loading shared libraries: libxcb-cursor.so.0: cannot open shared object file: No such file or directory
```

则表示系统缺少必要的库。在初次安装的 NOI Linux 2.0 下此类问题非常常见，你需要执行以下命令安装必要的库：

```bash
sudo apt update
sudo apt upgrade -y
sudo apt install libxcb-cursor0
```

您可以 #link("https://mirrors.ustc.edu.cn/help/ubuntu.html")[换源] 来加速安装过程。如果您不是很熟悉 Linux，Ubuntu 提供图形界面的换源功能，您可以在搜索引擎搜索 `Ubuntu 20.04 图形界面换源`（20.04 是 NOI Linux 2.0 的对应 Ubuntu 的版本号）相关关键词得到教程。

遇到问题请善用搜索引擎。

如还未配置编译器（如第一次运行）会弹出编译器配置弹窗。对于 C++，则需要选择 g++ 所在路径。一种简单的方法是直接使用 Dev-C++ 自带的编译器（一般在 `C:\Program Files (x86)\Dev-Cpp\MinGW64\bin\g++.exe`），但我们不建议你这么做。如果 g++ 已经在环境变量中，一般不需要手动选择。

我们提供了一份 #link("https://Project-LemonLime.github.io/Project_LemonLime/example.zip")[样例]，在 LemonLime 中将其打开后，点击左上角的 控制\>自动添加试题，即可看到一道默认的 `plus` 试题。

#image("./pics/autoaddproblem.png", alt:"Auto Add Problem")

添加试题后，点击试题，框选右侧详情中的 定向到标准输入 与 定向到标准输出。切到 选手 页，点击右下角的 刷新，即可自动加载选手。点击 测试全部，即可自动运行全部选手的代码。代码的运行会在沙箱下进行。

你可以自由地探索 LemonLime，并尝试添加试题、选手。试题数据需要放在比赛文件夹下的 `data` 目录，选手源代码需要放在比赛文件夹下的 `source` 目录。