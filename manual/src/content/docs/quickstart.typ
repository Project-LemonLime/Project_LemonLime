#import "../html.typ" : *
#show : html-style
#metadata(( title: "快速开始" ))<frontmatter>

= 快速开始

LemonLime 是为了 OI 比赛而生的基于 Lemon + LemonPlus 的轻量评测系统，具有三大桌面系统支持。

可以在 #link("https://github.com/Project-LemonLime/Project_LemonLime/releases/")[Github] 上下载最新版本。若因网络问题下载速度较慢，可以自行寻找镜像。

在 Windows 系统下可以解压压缩包，双击 `lemon.exe` 运行程序。

在 Linux 系统下可以在软件目录下通过终端运行 `chmod +x ./LemonLime-linux-...` 赋予权限，然后直接运行 `./LemonLime-linux-...` 启动。

遇到问题可查阅本手册「常见问题及回答」一节，同时请善用搜索引擎。

如还未配置编译器（如第一次运行）会弹出编译器配置弹窗。对于 C++，则需要选择 g++ 所在路径。如果 g++ 已经在环境变量中，一般不需要手动选择。建议勾选「添加建议配置」。

我们提供了一份 #link("https://Project-LemonLime.github.io/Project_LemonLime/example.zip")[样例]，在 LemonLime 中将其打开后，点击左上角的 控制\>自动添加试题，即可看到一道默认的 `plus` 试题。

#image("./pics/autoaddproblem.png", alt:"Auto Add Problem")

添加试题后，点击试题，框选右侧详情中的 定向到标准输入 与 定向到标准输出。切到 选手 页，点击右下角的 刷新，即可自动加载选手。点击 测试全部，即可自动运行全部选手的代码。

Linux 会对提交程序的执行使用沙箱。Windows 提供默认关闭的实验性 AppContainer 沙箱，可在高级编译器设置中按编译器勾选“实验性 Windows 沙箱”。启用后，评测期间会准备并复用选定运行环境的临时只读权限，评测结束后撤销；可以选择运行策略、额外只读目录和准备时间限制。所有 Windows 沙箱策略均限制为一个进程，禁止提交程序创建子进程。需要启动子进程的 Python 启动器和 Windows venv 会执行失败，应在编译器设置中选择基础 Python 等单进程运行入口。编译器与检查器当前仍使用宿主权限，macOS 当前仅提供资源监控；处理未知来源的代码时应使用专门的评测环境。

你可以自由地探索 LemonLime，并尝试添加试题、选手。试题数据需要放在比赛文件夹下的 `data` 目录，选手源代码需要放在比赛文件夹下的 `source` 目录。
