# LemonLime 用户手册

该文件夹储存 LemonLime 用户手册源代码。

用户手册使用 Typst 编写，源代码储存于 `manual/src/content` 目录中。

用户手册有两种分发模式：线上分发与线下分发。

线下分发需要手动编译 Typst 文件，在此之前，你需要下载以下字体：

- [Noto Serif CJK SC](https://github.com/notofonts/noto-cjk/releases/tag/Serif2.003)
- [Noto Sans CJK SC](https://github.com/notofonts/noto-cjk/releases/tag/Sans2.004)
- [LXGW WenKai](https://github.com/lxgw/LxgwWenKai/releases)
- [Courier Prime Code](https://quoteunquoteapps.com/courierprime/downloads/courier-prime-code.zip)

然后使用 Typst CLI 或者 Tinymist Typst 编译 `manual/src/content/manual.typ`，将生成出来的 pdf 重命名并放在 `manual/llmanual.pdf`。注意，需要开启 `--features html`。

线上分发使用使用 astro+[astro-typst](https://github.com/OverflowCat/astro-typst)+[starlight theme](https://starlight.astro.build/) 部署于线上。

`docs` 文件夹下的文件：

- `manual.typ`：手册文件，include 了所有文章，供 pdf 导出使用；
- `lib.typ`：手册文件的样式，供 pdf 导出使用；
- `html.typ`：网页文件的一些 hook，劫持了数学公式、图片、标题（网页中不显示标题，因为已经有一个了），其中图片通过调用 JSX 组件 `<Image>` 实现，未来 astro-typst 的 API 可能会更换，需要及时更改。由于 `<Image>` 必须提供 `alt`，在文件中插入图片时必须包含 `alt`。