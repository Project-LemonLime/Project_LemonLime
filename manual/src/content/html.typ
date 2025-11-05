#let target = dictionary(std).at("target", default: () => "paged")
#let html-style(content) = {
  show math.equation: it => context {
    if target() == "html" {
      show: if it.block { it => it } else { box }
      html.frame(it)
    } else {
      it
    }
  }
  show image: it => context {
    if target() == "html" {
      html.frame(it)
    } else {
      it
    }
  }
  show heading.where(level: 1): it => context {
    if target() != "html" {
      it
    }
  }

  content
}