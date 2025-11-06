# LemonLime 用户手册

该文件夹储存 LemonLime 用户手册源代码。

用户手册使用 Typst 编写，源代码储存于 `manual/src/content` 目录中。

用户手册有两种分发模式：线上分发与线下分发。

线下分发需要手动编译 Typst 文件，在此之前，你需要下载以下字体：

- [Noto Serif CJK SC](https://github.com/notofonts/noto-cjk/releases/tag/Serif2.003)
- [Noto Sans CJK SC](https://github.com/notofonts/noto-cjk/releases/tag/Sans2.004)
- [LXGW WenKai](https://github.com/lxgw/LxgwWenKai/releases)
- [Courier Prime Code](https://quoteunquoteapps.com/courierprime/downloads/courier-prime-code.zip)

然后使用 Typst CLI 或者 Tinymist Typst 编译 `manual/src/content/manual.typ`，将生成出来的 pdf 重命名并放在 `manual/llmanual.pdf`。

线上分发使用使用 astro+[astro-typst](https://github.com/OverflowCat/astro-typst)+[starlight theme](https://starlight.astro.build/) 部署于线上。
