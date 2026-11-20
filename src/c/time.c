#include "time.h"
#include "text.h"
#include "watchface.h"

static char time_temp[8];

TimeConfig time_config(const GColor color_primary, const GColor color_secondary, const GColor color_accent, const TimeFont font) {
  TimeConfig config;
  config.color_primary = color_primary;
  config.color_secondary = color_secondary;
  config.color_accent = color_accent;
  config.font = font;
  return config;
}

GRect time_rect(TimeSection *section, int16_t x, int16_t y, int16_t w, int16_t h) {
  return GRect(
    section->bounds.origin.x + x,
    section->bounds.origin.y + y,
    w, h
  );
}

void time_load(TimeSection *section, Layer *root_layer, const TimeConfig config) {
  GRect bounds = layer_get_bounds(root_layer);
  // Create time segment
  //APP_LOG(APP_LOG_LEVEL_DEBUG, "Time rect: (%d, %d, %d, %d)", 0, watchface_bar_height(), bounds.size.w, watchface_time_height());
  section->bounds = GRect(0, watchface_bar_height(), bounds.size.w, watchface_time_height());
  time_load_settings(section, config);
}

void time_load_settings(TimeSection *section, const TimeConfig config) {
  section->redraw_text = true;
  section->config = config;
  if (config.font == TIME_FONT_BEBAS) {
    time_load_font(section, RESOURCE_ID_BEBAS);
  } else if (config.font == TIME_FONT_BARLOW) {
    time_load_font(section, RESOURCE_ID_BARLOW);
  } else {
    time_load_font(section, RESOURCE_ID_TEKO);
  }
}

void time_load_font(TimeSection *section, const uint32_t resource_id) {
  if (section->font) {
    ffont_destroy(section->font);
    section->font = NULL;
  }
  section->font = ffont_create_from_resource(resource_id);
}

void time_unload(TimeSection *section) {
  if (section->font) {
    ffont_destroy(section->font);
    section->font = NULL;
  }
}

void time_update(TimeSection *section, struct tm *tick_time, TimeUnits units_changed) {
  ClaySettings *s = settings_get();
  if (s->time_format == TIME_FORMAT_12H) {
    strftime(time_temp, sizeof(time_temp), "%I:%M", tick_time);
    if (section->buffer[0] == '0') {
        memmove(time_temp, time_temp + 1, strlen(time_temp));
    }
  } else {
    strftime(time_temp, sizeof(time_temp), "%H:%M", tick_time);
  }
  if (strcmp(section->buffer, time_temp) != 0) {
    strncpy(section->buffer, time_temp, sizeof(section->buffer));
    time_refresh(section, false);
  }
}

void time_update_layer(TimeSection *section, Layer *layer, GContext *ctx) {
  if (!section->redraw_full && !section->redraw_text) {
    return;
  }
  //APP_LOG(APP_LOG_LEVEL_DEBUG, "time_update_layer");
  section->redraw_text = false;
  const GRect bounds = section->bounds;
  const uint16_t px = 4; // Horizontal text padding
  const uint16_t py = 8; // Vertical text padding
  const uint16_t width = bounds.size.w - px * 2;
  const GRect text_bounds = time_rect(section, px, py, width, bounds.size.h - py * 2);
  if (section->redraw_full) {
    // Full redraw -> Fill the whole time area and redraw accents as well
    section->redraw_full = false;
    // Render background
    graphics_context_set_fill_color(ctx, section->config.color_secondary);
    graphics_fill_rect(ctx, bounds, 0, GCornerNone);
    // Render accent lines
    const uint16_t apx = 4; // Horizontal accent padding
    const uint16_t apy = 4; // Vertical accent padding
    const uint16_t accent_height = watchface_accent_height();
    const uint16_t accent_width = bounds.size.w - apx * 2;
    const GRect accent_top = time_rect(section, apx, apy - accent_height, accent_width, accent_height);
    const GRect accent_bottom = time_rect(section, apx, bounds.size.h - apy, accent_width, accent_height);
    graphics_context_set_fill_color(ctx, section->config.color_accent);
    graphics_fill_rect(ctx, accent_top, 0, GCornerNone);
    graphics_fill_rect(ctx, accent_bottom, 0, GCornerNone);
  } else {
    // Partial redraw -> Just fill the previous text area and redraw that
    graphics_context_set_fill_color(ctx, section->config.color_secondary);
    graphics_fill_rect(ctx, text_bounds, 0, GCornerNone);
  }
  // Render text
  if (section->font) {
    text_render_fit(ctx, text_bounds, section->buffer, section->font, section->config.color_primary, GTextAlignmentCenter);
  } else {
    // TODO: Implement fallback using system font
  }
}

void time_refresh(TimeSection *section, bool full_refresh) {
  if (full_refresh) {
    section->redraw_full = true;
  } else {
    section->redraw_text = true;
  }
  watchface_redraw();
}