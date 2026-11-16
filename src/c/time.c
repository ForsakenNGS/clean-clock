#include "time.h"
#include "text.h"
#include "watchface.h"

static char time_temp[8];

static void time_update_layer(Layer *layer, GContext *ctx) {
  TimeSectionData *d = (TimeSectionData*)layer_get_data(layer);
  GRect bounds = layer_get_bounds(layer);
  const uint16_t px = 4;
  const uint16_t py = 4;
  const uint16_t left = px;
  const uint16_t width = bounds.size.w - px * 2;
  // Render background
  graphics_context_set_fill_color(ctx, d->config.color_secondary);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);
  // Render accent lines
  const uint16_t accent_height = watchface_accent_height();
  const GRect accent_top = GRect(left, py - accent_height, width, accent_height);
  const GRect accent_bottom = GRect(left, bounds.size.h - py, width, accent_height);
  graphics_context_set_fill_color(ctx, d->config.color_accent);
  graphics_fill_rect(ctx, accent_top, 0, GCornerNone);
  graphics_fill_rect(ctx, accent_bottom, 0, GCornerNone);
  // Render text
  if (d->font) {
    GRect text_bounds = GRect(left, watchface_bar_height() + py, width, bounds.size.h - py * 2);
    text_render_fit(ctx, text_bounds, d->buffer, d->font, d->config.color_primary, GTextAlignmentCenter);
  } else {
    // TODO: Implement fallback using system font
  }
}

TimeConfig time_config(const GColor color_primary, const GColor color_secondary, const GColor color_accent, const TimeFont font) {
  TimeConfig config;
  config.color_primary = color_primary;
  config.color_secondary = color_secondary;
  config.color_accent = color_accent;
  config.font = font;
  return config;
}

void time_load(TimeSection *section, Layer *root_layer, const TimeConfig config) {
  GRect bounds = layer_get_bounds(root_layer);
  // Create time segment
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Time rect: (%d, %d, %d, %d)", 0, watchface_bar_height(), bounds.size.w, watchface_time_height());
  section->layer = layer_create_with_data(
    GRect(0, watchface_bar_height(), bounds.size.w, watchface_time_height()),
    sizeof(TimeSectionData)
  );
  section->data = (TimeSectionData*)layer_get_data(section->layer);
  time_load_settings(section, config);
  layer_set_update_proc(section->layer, time_update_layer);
  layer_add_child(root_layer, section->layer);
}

void time_load_settings(TimeSection *section, const TimeConfig config) {
  section->data->config = config;
  if (config.font == TIME_FONT_BEBAS) {
    time_load_font(section, RESOURCE_ID_BEBAS);
  } else if (config.font == TIME_FONT_BARLOW) {
    time_load_font(section, RESOURCE_ID_BARLOW);
  } else {
    time_load_font(section, RESOURCE_ID_TEKO);
  }
}

void time_load_font(TimeSection *section, const uint32_t resource_id) {
  if (section->data->font) {
    ffont_destroy(section->data->font);
    section->data->font = NULL;
  }
  section->data->font = ffont_create_from_resource(resource_id);
}

void time_unload(TimeSection *section) {
  if (section->data->font) {
    ffont_destroy(section->data->font);
    section->data->font = NULL;
  }
  layer_destroy(section->layer);
}

void time_update(TimeSection *section, struct tm *tick_time, TimeUnits units_changed) {
  ClaySettings *s = settings_get();
  if (s->time_format == TIME_FORMAT_12H) {
    strftime(time_temp, sizeof(time_temp), "%I:%M", tick_time);
    if (section->data->buffer[0] == '0') {
        memmove(time_temp, time_temp + 1, strlen(time_temp));
    }
  } else {
    strftime(time_temp, sizeof(time_temp), "%H:%M", tick_time);
  }
  if (strcmp(section->data->buffer, time_temp) != 0) {
    strncpy(section->data->buffer, time_temp, sizeof(section->data->buffer));
    layer_mark_dirty(section->layer);
  }
}

void time_refresh(TimeSection *section) {
  layer_mark_dirty(section->layer);
}