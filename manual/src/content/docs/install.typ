#import "../html.typ" : *
#show : html-style
#metadata(( title: "安装" ))<frontmatter>

= 安装

LemonLime 在上文中提及的 #link("https://github.com/Project-LemonLime/Project_LemonLime")[仓库地址] 不仅用来存储源代码，也用来实时发布软件最新版本。

== 支持的系统

LemonLime 支持 Windows、Linux 以及 macOS。

== 安装方法

=== Windows 和 macOS

在 #link("https://github.com/Project-LemonLime/Project_LemonLime/releases")[发行页面] 下载相应的压缩包。

不过如果你获得了源码，可以通过 QtCreator 来编译 LemonLime。

=== Linux

这里以 Manjaro 为例，毕竟 Linux 安装时几乎只有命令名字的区别。

更多的系统请到 README.md 查看。

首先你得安装一些依赖。

```bash
$ sudo pacman -S gcc ninja qt5-base cmake
```

注意 Manjaro 的 gcc 包含了 `g++`。

然后用某种途径获得源代码。比如使用 git：

```bash
$ git clone https://github.com/Project-LemonLime/Project_LemonLime.git
```

进入源代码的目录。

```bash
$ cd <源代码的目录>
```

然后依次执行：

```bash
$ cmake . -GNinja -DCMAKE_BUILD_TYPE=Release
$ cmake --build . --parallel $(nproc)
```

获得可执行文件 lemon。打开 lemon 就可以运行 LemonLime 了。

也可以执行 `$ cmake –install .` 将其安装到系统中，默认安装位置位于 `/usr/local`

同时，在装有 `dpkg/rpm` 的 Linux 系统上 可为 CMake 附加 `-DBUILD_DEB=ON`
或者 `-DBUILD_RPM=ON` 参数直接生成 Deb/RPM 包。即：

```bash
$ cmake . -GNinja -DCMAKE_BUILD_TYPE=Release -DBUILD_DEB=ON
$ cmake --build . --parallel $(nproc)
```

推荐在直接生成包的时候同时附加 `-DEMBED_TRANSLATIONS=OFF -DEMBED_DOCS=OFF` 参数关闭翻译文件和手册文件的嵌入。