#include "text.h"

void text_render_fit(GContext *ctx, GRect bounds, char* text, FFont* font, GColor color, GTextAlignment alignment) {
  FContext fctx;
  fctx_init_context(&fctx, ctx);
  fixed_t font_size = bounds.size.h;
  // Set full size and check resulting width
  fctx_set_text_cap_height(&fctx, font, font_size);
  fixed_t full_width = fctx_string_width(&fctx, text, font) / 16;
  // Calculate fitting size if overflowing
  if (full_width > bounds.size.w) {
    font_size = font_size * bounds.size.w / full_width;
    fctx_set_text_cap_height(&fctx, font, font_size);
  }
  // Calculate offset
  fixed_t offset_y = font_size + (bounds.size.h - font_size) / 2;
  fctx_set_fill_color(&fctx, color);
  fctx_set_offset(&fctx, FPointI(
    bounds.origin.x + bounds.size.w / 2,
    bounds.origin.y + offset_y
  ));
  fctx_begin_fill(&fctx);
  fctx_draw_string(&fctx, text, font, alignment, FTextAnchorBaseline);
  fctx_end_fill(&fctx);
  fctx_deinit_context(&fctx);
}