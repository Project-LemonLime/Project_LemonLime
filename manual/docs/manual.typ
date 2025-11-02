#import "./lib.typ": *

#show: project.with(
  title: "LemonLime 用户手册",
  author: "浮尘ii*, iotang, Coelacanthus",
  date: auto,
)

#image("pics/icon.png", width: 30%)

#align(center)[
为了 OI 比赛而生的基于 Lemon + LemonPlus 的轻量评测系统

三大桌面系统支持
]

#pagebreak()

#outline()

#pagebreak()

#include "./quickstart.typ"

#include "./intro.typ"

#include "./install.typ"

#include "./settings.typ"

#include "./contest.typ"

#include "./judge.typ"

#include "./statistics.typ"

#include "./issue.typ"
