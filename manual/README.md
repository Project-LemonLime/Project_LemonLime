## LemonLime 用户手册

该文件夹储存 LemonLime 用户手册源代码。

用户手册使用 Typst 编写，源代码储存于 `docs` 目录中。

用户手册有两种分发模式：线上分发与线下分发。

线下分发需要手动编译 Typst 文件，在此之前，你需要下载以下字体：

- [Noto Serif CJK SC](https://github.com/notofonts/noto-cjk/releases/tag/Serif2.003)
- [Noto Sans CJK SC](https://github.com/notofonts/noto-cjk/releases/tag/Sans2.004)
- [LXGW WenKai](https://github.com/lxgw/LxgwWenKai/releases)
- [Courier Prime Code](https://quoteunquoteapps.com/courierprime/downloads/courier-prime-code.zip)

然后使用 Typst CLI 或者 Tinymist Typst 编译 `manual/docs/manual.typ`，将生成出来的 pdf 重命名并放在 `manual/llmanual.pdf`。

线上分发使用 pandoc 将 typst 文件转换为 md 文件，然后使用 [material for mkdocs](https://squidfunk.github.io/mkdocs-material/) 部署于线上。

转换工具存储于 `scripts/build_html.py` 中，需要 pandoc。在项目根目录下使用 `python ./manual/scripts/build_html.py` 调用。生成出来的文件在 `site` 目录下。

`docs` 文件夹已经使用 `.gitignore` 忽略除了 `index.md`（需要特殊处理主页） 外的 `.md` 文件。