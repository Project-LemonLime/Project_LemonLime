#let font = (
  zh_shusong: "Noto Serif CJK SC",
  zh_zhongsong: "Noto Serif CJK SC",
  zh_kai: "LXGW WenKai",
  zh_hei: "Noto Sans CJK SC",
  en_sans_serif: "New Computer Modern",
  en_serif: "New Computer Modern",
  en_code: "Courier Prime Code",
)

#let font-size = (
  s2: 18pt, // 二号
  n3: 16pt, // 三号
  s3: 15pt, // 小三
  n4: 14pt, // 四号
  s4: 12pt, // 小四
  n5: 10.5pt, // 五号
  s5: 9pt, // 小五
)

#let config = (
  // 字号设置
  text-size: font-size.n5,
  author-size: font-size.n5,
  title-size: font-size.s2,
  title1-size: font-size.s3,
  title2-size: font-size.n4,
  title3-size: font-size.s4,
  // 字体设置
  title-font: (font.en_serif, font.zh_hei),
  author-font: (font.en_sans_serif, font.zh_shusong),
  body-font: (font.en_serif, font.zh_shusong),
  heading-font: (font.en_serif, font.zh_zhongsong),
  caption-font: (font.en_serif, font.zh_kai),
  header-font: (font.en_serif, font.zh_kai),
  strong-font: (font.en_serif, font.zh_hei),
  emph-font: (font.en_serif, font.zh_kai),
  raw-font: (font.en_code, font.zh_hei),
  // 间距设置
  spacing: 1.5em,
  leading: 1.0em,
  indent: 2em,
  small-space: 1em,
  block-space: 0.75em,
  // 颜色设置
  raw-color: rgb("#f0f0f0"),
  // 列表样式
  list-marker: ([•], [◦], [▶]),
  enum-numbering: ("1.", "(1)", "①", "a."),
)

#let title-state = state("document-title", "")

// 解决首段缩进问题的空白段
#let fake-par = {
  par(box())
  v(-config.spacing)
}

#let header-style(heading) = {
  set text(font: config.header-font)
  let title = title-state.get()
  grid(
    columns: (1fr, 1fr),
    align(left, title), align(right, heading),
  )
  v(-1.2em)
  line(stroke: 1pt + gray, length: 100%)
}

#let prev-header = context {
  let headings = query(heading.where(level: 1).before(here()))
  if headings.len() == 0 {
    return
  }
  let level = counter(heading.where(level: 1)).display("一")
  let heading = level + h(config.small-space) + headings.last().body
  header-style(heading)
}

#let next-header = context {
  let headings = query(heading.where(level: 1).after(here()))
  if headings.len() == 0 {
    return
  }
  let count = counter(heading.where(level: 1)).get().first() + 1
  let level = numbering("一", count)
  let heading = level + h(config.small-space) + headings.first().body
  header-style(heading)
}

// 标题
#let make-title(
  title: "",
  author: "",
  date: none,
) = {
  align(center)[
    #block(
      text(
        font: config.title-font,
        weight: "bold",
        config.title-size,
        title,
      ),
    )
    #v(0.5em)
  ]

  if author != "" {
    set text(config.author-size, font: config.author-font)
    align(center, author)
  }

  if date != none {
    date = if date == auto {
      datetime.today().display("[year]年[month]月[day]日")
    } else {
      date
    }
    set text(config.author-size, font: config.author-font)
    align(center, date)
  }
}

#let project(
  title: "",
  author: "",
  date: none,
  body,
) = {
  title-state.update(title)

  // 文档设置
  set document(author: author, title: title, date: date)

  set page(
    numbering: "1",
    number-align: center,
    header: prev-header,
  )

  set heading(numbering: "1.1")
  set text(
    font: config.body-font,
    lang: "zh",
    region: "cn",
    size: config.text-size,
  )
  set par(
    first-line-indent: config.indent,
    justify: true,
    leading: config.leading,
    spacing: config.spacing,
  )
  set enum(
    indent: config.indent,
    full: true,
    numbering: (..n) => {
      n = n.pos()
      let level = n.len()
      let number = config.enum-numbering.at(level - 1, default: "1.")
      numbering(number, ..n.slice(level - 1))
    },
  )
  set list(
    indent: config.indent,
    marker: config.list-marker,
  )
  set math.equation(numbering: "(1)")
  set underline(evade: false)

  show heading: it => {
    set text(font: config.heading-font)
    let body = if it.numbering != none {
      counter(heading).display() + h(config.small-space) + it.body
    } else {
      it.body
    }
    box(width: 100%, body)
  }

  show heading.where(level: 1): it => {
    set align(center)
    set heading(numbering: "一")
    set text(config.title1-size)
    it
  }

  show heading.where(level: 2): it => {
    v(-0.75em)
    set text(config.title2-size)
    it
    v(-0.25em)
  }

  show heading.where(level: 3): it => {
    v(-1em)
    set text(config.title3-size)
    it
  }

  show figure: it => {
    set block(breakable: true)
    set text(font: config.caption-font)
    it + fake-par
  }
  show figure.where(kind: table): set figure.caption(position: top)
  show table: it => {
    set text(font: config.body-font)
    it + fake-par
  }
  show image: it => it + fake-par
  show list: it => {
    set list(indent: 0em)
    set enum(indent: 0em)
    it + fake-par
  }
  show enum: it => {
    set list(indent: 0em)
    set enum(indent: 0em)
    it + fake-par
  }
  show terms: it => {
    set text(font: config.caption-font)
    it + fake-par
  }

  show strong: set text(font: config.strong-font)
  show emph: set text(font: config.emph-font)
  show ref: set text(red)
  show link: it => {
    set text(blue)
    underline(it)
  }

  show raw.where(block: false): it => {
    set text(font: config.raw-font)
    box(
      fill: config.raw-color,
      inset: (x: 3pt, y: 0pt),
      outset: (x: 0pt, y: 3pt),
      radius: 2pt,
      it,
    )
  }
  show raw.where(block: true): it => {
    set text(font: config.raw-font)
    set block(
      width: 100%,
      fill: config.raw-color,
      outset: (x: 0pt, y: 4pt),
      inset: (x: 8pt, y: 4pt),
      radius: 4pt,
    )
    it + fake-par
  }

  make-title(
    title: title,
    author: author,
    date: date,
  )

  body
}