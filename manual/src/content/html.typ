#let html-style(content) = {
  let jsx = s => html.elem("script", attrs: ("data-jsx": s))
  let num = state("imageNum", 1)
  jsx("import { Image } from 'astro:assets'")
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
      num.update(num => num + 1)
      jsx("import Img" + str(num.get()) + " from '" + it.source + "'")
      jsx("<Image src={Img" + str(num.get()) + "} alt='" + it.alt + "' />")
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