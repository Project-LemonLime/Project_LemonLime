#import "../html.typ" : *
#show : html-style
#metadata(( title: "常见问题及回答" ))<frontmatter>

= 常见问题及回答

在使用中出现任何问题，可以在 #link("https://github.com/Project-LemonLime/Project_LemonLime")[Github 仓库] 中的 Issue 提出。

此节内容长期更新，更多问题欢迎提出。

== Windows 无法启动

尝试安装同目录下的 `vc_redist.x64.exe` 解决。

== Linux 无法启动

如果启动时出现：

```bash
error while loading shared libraries: libxcb-cursor.so.0: cannot open shared object file: No such file or directory
```

你需要执行以下命令安装必要的库：

```bash
sudo apt update
sudo apt upgrade -y
sudo apt install libxcb-cursor0
```

你可以 #link("https://mirrors.ustc.edu.cn/help/ubuntu.html")[换源] 来加速安装过程。

== WSL 无法启动

WSL2 的 Kernel 自带的运行库不全，不包含 bubblewrap，无法创建子进程。你需要执行以下命令安装必要的库：

```bash
sudo apt install bubblewrap
```

== 测评时使用更多栈空间

Windows 平台下可以在 `g++` 编译时添加 `-Wl,--stack=2147483647` 命令来开启约 2048 MB 栈空间。

Linux 平台下的栈空间限制和题目的内存限制相同。

== 第一个点 TLE

测评时请将杀毒软件关闭。

== Mac 系统随机 TLE

详见 #link("https://github.com/Project-LemonLime/Project_LemonLime/issues/120")[issues/120]。
