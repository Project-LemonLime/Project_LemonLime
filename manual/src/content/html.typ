#let html-style(content) = {
  let jsx = s => html.elem("script", attrs: ("data-jsx": s))

  jsx("import { Image } from 'astro:assets'")
  let image-num = state("image num", 0)
  show image: it => context {
    if target() == "html" {
      image-num.update(image-num => image-num + 1)
      jsx("import Img" + str(image-num.get()) + " from '" + it.source + "'")
      jsx("<Image src={Img" + str(image-num.get()) + "} alt='" + it.alt + "' />")
    } else {
      it
    }
  }

  show raw.where(block: true): it => {
    if target() == "html" {
      jsx("```" + it.lang + "\n" + it.text + "\n```")
    } else {
      it
    }
  }

  show math.equation: it => context {
    if target() == "html" {
      show: if it.block { it => it } else { box }
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