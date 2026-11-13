#!/usr/bin/env python3
"""Convert TTF fonts to compiled pebble-fctx .ffont resources.

This is a one-step generator: for each TTF in fonts/ it:
  1. builds an intermediate SVG font description into build/fonts-svg/,
  2. runs pebble-fctx-compiler to produce a compiled .ffont into resources/.

Usage:
    python3 scripts/ttf_to_svg_font.py

The compiled .ffont files are consumed at runtime via ffont_create_from_resource()
in src/c/clean-clock.c and declared as raw resources in package.json.
"""
import os
import subprocess
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
RESOURCES_DIR = os.path.join(PROJECT_ROOT, "resources")

FCTX_COMPILER = os.path.join(PROJECT_ROOT, "node_modules", ".bin", "fctx-compiler")

# Characters needed for the digital time display.
CHARS = "0123456789:"

# TTF source -> (font id used in <font id="...">)
# Add new fonts here; the output resource basename is derived from the ttf name.
FONT_SOURCES = [
    ("Teko-Light.ttf", "Teko"),
    ("EBGaramond.ttf", "EBGaramond"),
    ("LECO 1976 Regular.ttf", "LECO1976"),
]


def svg_name(ttf_name):
    """Derive the intermediate SVG file basename from a ttf file name."""
    return os.path.splitext(ttf_name)[0].replace(" ", "").replace("'", "") + ".svg"


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


def compile_svg(svg_path):
    """Run pebble-fctx-compiler on an SVG, writing a .ffont into resources/."""
    if not os.path.exists(FCTX_COMPILER):
        print("ERROR: fctx-compiler not found at %s" % FCTX_COMPILER, file=sys.stderr)
        print("Install it: npm install pebble-fctx-compiler", file=sys.stderr)
        return False
    os.makedirs(RESOURCES_DIR, exist_ok=True)
    cmd = [FCTX_COMPILER, svg_path]
    print("Running: %s" % " ".join(cmd))
    result = subprocess.run(cmd, cwd=PROJECT_ROOT)
    if result.returncode != 0:
        print("ERROR: fctx-compiler failed for %s (exit %d)" % (svg_path, result.returncode), file=sys.stderr)
        return False
    return True


def main():
    if not os.path.isdir(FONTS_DIR):
        print("ERROR: fonts directory not found: %s" % FONTS_DIR, file=sys.stderr)
        sys.exit(1)

    os.makedirs(SVG_OUT_DIR, exist_ok=True)

    any_failed = False
    for ttf_name, font_id in FONT_SOURCES:
        ttf_path = os.path.join(FONTS_DIR, ttf_name)
        if not os.path.exists(ttf_path):
            print("WARNING: not found, skipping: %s" % ttf_path, file=sys.stderr)
            continue
        svg_path = os.path.join(SVG_OUT_DIR, svg_name(ttf_name))
        build_svg_font(ttf_path, svg_path, font_id)
        if not compile_svg(svg_path):
            any_failed = True

    print()
    if any_failed:
        print("One or more fonts failed to compile.", file=sys.stderr)
        sys.exit(1)
    print("Done. Compiled .ffont resources are in %s" % RESOURCES_DIR)


if __name__ == "__main__":
    main()
