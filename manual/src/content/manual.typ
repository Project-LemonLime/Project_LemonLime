#import "./lib.typ": *

#show: project.with(
  title: "LemonLime 用户手册",
  author: "浮尘ii*, iotang, Coelacanthus",
  date: auto,
)

#image("./docs/pics/icon.png", width: 30%)

#align(center)[
为了 OI 比赛而生的基于 Lemon + LemonPlus 的轻量评测系统

三大桌面系统支持
]

#pagebreak()

#outline()

#pagebreak()

#include "./docs/quickstart.typ"

#include "./docs/intro.typ"

#include "./docs/install.typ"

#include "./docs/settings.typ"

#include "./docs/contest.typ"

#include "./docs/judge.typ"

#include "./docs/statistics.typ"

#include "./docs/issue.typ"
