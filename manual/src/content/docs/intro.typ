#import "../html.typ" : *
#show : html-style
#metadata(( title: "简介" ))<frontmatter>

= 简介

== 历史

#link("https://github.com/Project-LemonLime/Project_LemonLime")[LemonLime] 项目是 #link("https://github.com/Dust1404/Project_LemonPlus")[LemonPlus] 的二次开发。

#v(2em)

2011 年，Jia Zhipeng 开发完成了 Lemon，一个开源的评测工具。

虽然我们并不知道他确切的动机，

——是想弥补 Linux 下没有评测工具的缺陷，还是想造出一个 Cena 的对手，

不管怎样，他的愿望达成了，Lemon 的历史也从此开始。

现在（2020 年）他已经是一名得克萨斯大学奥斯汀分校的 2 年级博士生了，祝他的学业生涯顺利。

#v(1em)

2018 年，在 浮尘ii\* 的努力下，Lemon 进化为 LemonPlus。

LemonPlus 成功兼容了 Qt 5，

并且新增了适合新时代 OI 的功能，移除了一些不稳定的、过时的功能。

奈何 OI 残酷，岁月更迁，物是人非，浮尘ii\* 在 2019 年退役了，

LemonPlus 的开发也相应地结束了。

#v(1em)

2019 年，iotang 开始魔改 Lemon。

起初，是想给 Lemon 增添一点用户体验——

添加更多的颜色、优化界面，以及修改令人崩溃的重测逻辑，

但是误打误撞地在 Github 上发现了 LemonPlus。

可怜的 iotang 当时还不会用 Pull Request，所以 LemonLime 诞生了。

LemonLime 继承了 LemonPlus 和 LemonMt（这个大概只有长郡人才会知道）的优点。

#v(1em)

放眼望去，Lemon 的历史就像是传火。

Lemon 开发要想继续，就必须要燃烧 Oier 的灵魂。

2020 年，随着政策巨变，iotang 也在退役的路上了。

虽然退役并不代表开发停止，但是开发 Lemon 的热情将再也不能体会到了。

也只能可惜 iotang 不够强了吧。

#v(1em)

2020 年，因为功能欠缺的 qmake，

Ceolacanthus 开始了把 LemonLime 从 QMake 迁移到 CMake 的路程。

又因为不想每次都编译，她开始改造 LemonLime 的 CI，

试图完成自动编译和打包。

2020 年底，她成功的让 LemonLime 兼容了 Qt6。

2021 年，Alphagocc 让 LemonLime 重新支持了多线程评测和新的比赛文件格式

当然，这一切都还没完成。

#v(1em)

本篇用户手册是仿照 LemonPlus 的用户手册进行编写的。

== Lemon 与开源意志

LemonLime 使用 #link("https://www.gnu.org/licenses/gpl-3.0.html")[GPLv3] 协议。也就是说，LemonLime 是自由软件。

自由软件是什么？为什么 LemonLime 选择成为自由软件？

自由软件意味着使用者有运行、复制、发布、研究、修改和改进该软件的自由。

自由软件是权利问题，不是价格问题。

要理解这个概念，你应该考虑"free"是"言论自由（free speech）"中的"自由"，

而不是"免费啤酒（free beer）"中的"免费"。

更精确地说，自由软件赋予软件使用者四项基本自由：

- 不论目的为何，有运行该软件的自由（自由之零）。

- 有研究该软件如何工作以及按需改写该软件的自由（自由之一）。

- 有重新发布拷贝的自由，这样你可以借此来敦亲睦邻（自由之二）。

- 有向公众发布改进版软件的自由（自由之三），这样整个社群都可因此受惠。

不管是 Lemon，还是它的后继者 LemonPlus 和 LemonLime，

都恪守着自由软件的意志。

这是 LemonPlus 和 LemonLime 诞生的保障，

也是 Lemon 不断延续自己的生命的保障。

#align(center)[
Lemon，以及它的后继者们，拥抱开源。

我们希望，Lemon 可以为全世界的 OIer 带来福祉。

让我们一起，把 Lemon 变得更好。
]

== 版本兼容性

LemonLime 近乎完全兼容 LemonPlus，出问题的地方在子任务依赖（为了支持 0 分测试点的让步，不过这个问题将会被解决）。不过这个问题在使用对应平台重新测试后就可以自动解决。

LemonLime 完全兼容 Lemon。

LemonLime 主要使用 Qt6 编写，目前支持的最低版本为 Qt 6.8。由于 Qt5 的兼容性不再保证，请在编译时开启 `-DLEMON_QT6=ON` 参数，若不开启可能无法编译（该参数计划在未来移除）。

LemonLime 可以使用 `GCC`, `Clang` 和 `MSVC` 进行编译