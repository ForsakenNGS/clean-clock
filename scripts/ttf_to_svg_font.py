#!/usr/bin/env python3
"""Convert TTF fonts to SVG font format for pebble-fctx-compiler.

Usage:
    python3 scripts/ttf_to_svg_font.py

This produces an SVG font file per TTF in fonts/ into build/fonts-svg/.
Compile each SVG with:
    ./node_modules/.bin/fctx-compiler <svg file>
which writes a .ffont binary into resources/ for use with pebble-fctx.
"""
import os
import sys

# Ensure fontTools is importable when run from any cwd.
try:
    from fontTools.ttLib import TTFont
    from fontTools.pens.svgPathPen import SVGPathPen
except ImportError:
    print("fontTools is required: pip3 install fontTools", file=sys.stderr)
    sys.exit(1)

PROJECT_ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
FONTS_DIR = os.path.join(PROJECT_ROOT, "fonts")
SVG_OUT_DIR = os.path.join(PROJECT_ROOT, "build", "fonts-svg")

# Characters needed for the digital time display.
CHARS = "0123456789:"

# TTF source -> (svg output, font id used in <font id="...">)
FONT_SOURCES = [
    ("EBGaramond.ttf", "EBGaramond"),
    ("LECO 1976 Regular.ttf", "LECO1976"),
]


def build_svg_font(ttf_path, svg_path, font_id):
    font = TTFont(ttf_path)
    head = font["head"]
    hhea = font["hhea"]
    os2 = font["OS/2"]
    cmap = font.getBestCmap()
    glyph_set = font.getGlyphSet()
    upem = head.unitsPerEm
    ascent = hhea.ascent

    cap_height = getattr(os2, "sCapHeight", None)
    if not cap_height or cap_height <= 0:
        cap_height = int(ascent * 0.7)

    lines = []
    lines.append("<?xml version=\"1.0\" encoding=\"UTF-8\"?>")
    lines.append("<svg xmlns=\"http://www.w3.org/2000/svg\">")
    lines.append("  <defs>")
    lines.append('    <font id="%s" horiz-adv-x="%d">' % (font_id, upem))
    lines.append(
        '      <font-face'
        ' font-family="%s"'
        ' units-per-em="%d"'
        ' ascent="%d"'
        ' descent="%d"'
        ' cap-height="%d"'
        ' unicode-range="U+0-10FFFF"' % (
            font_id, upem, hhea.ascent, hhea.descent, cap_height))
    lines.append("      />")

    for ch in CHARS:
        code = ord(ch)
        name = cmap.get(code)
        if name is None:
            continue
        glyph = glyph_set[name]
        adv = glyph.width
        spen = SVGPathPen(glyph_set)
        glyph.draw(spen)
        d = spen.getCommands()
        if not d:
            continue
        lines.append(
            '      <glyph unicode="%s" glyph-name="%s" horiz-adv-x="%d" d="%s" />'
            % (ch, name, adv, d))

    lines.append("    </font>")
    lines.append("  </defs>")
    lines.append("</svg>")

    os.makedirs(os.path.dirname(svg_path), exist_ok=True)
    with open(svg_path, "w") as f:
        f.write("\n".join(lines) + "\n")
    print("Wrote %s" % svg_path)


def main():
    os.makedirs(SVG_OUT_DIR, exist_ok=True)
    for ttf_name, font_id in FONT_SOURCES:
        ttf_path = os.path.join(FONTS_DIR, ttf_name)
        if not os.path.exists(ttf_path):
            print("WARNING: not found, skipping: %s" % ttf_path, file=sys.stderr)
            continue
        base = os.path.splitext(ttf_name)[0].replace(" ", "").replace("'", "")
        svg_path = os.path.join(SVG_OUT_DIR, base + ".svg")
        build_svg_font(ttf_path, svg_path, font_id)

    print()
    print("Next: compile each SVG with pebble-fctx-compiler into resources/:")
    print("  ./node_modules/.bin/fctx-compiler build/fonts-svg/EBGaramond.svg")
    print("  ./node_modules/.bin/fctx-compiler build/fonts-svg/LECO1976.svg")
    print("This writes resources/EBGaramond.ffont and resources/LECO1976.ffont.")


if __name__ == "__main__":
    main()
