#include "time.h"
#include "text.h"
#include "watchface.h"

static char time_temp[8];

static void time_update_layer(Layer *layer, GContext *ctx) {
  TimeSectionData *d = (TimeSectionData*)layer_get_data(layer);
  if (!d->font) return;

  GRect bounds = layer_get_bounds(layer);
  graphics_context_set_fill_color(ctx, d->color_secondary);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);
  GRect text_bounds = GRect(4, watchface_bar_height() + 2, bounds.size.w - 8, bounds.size.h - 4);
  text_render_fit(ctx, text_bounds, d->buffer, d->font, d->color_primary, GTextAlignmentCenter);
}

void time_load(TimeSection *section, Layer *root_layer, const GColor color_primary, const GColor color_secondary, const TimeFont font) {
  GRect bounds = layer_get_bounds(root_layer);
  // Create time segment
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Time rect: (%d, %d, %d, %d)", 0, watchface_bar_height(), bounds.size.w, watchface_time_height());
  section->layer = layer_create_with_data(
    GRect(0, watchface_bar_height(), bounds.size.w, watchface_time_height()),
    sizeof(TimeSectionData)
  );
  section->data = (TimeSectionData*)layer_get_data(section->layer);
  time_load_settings(section, color_primary, color_secondary, font);
  layer_set_update_proc(section->layer, time_update_layer);
  layer_add_child(root_layer, section->layer);
}

void time_load_settings(TimeSection *section, const GColor color_primary, const GColor color_secondary, const TimeFont font) {
  section->data->color_primary = color_primary;
  section->data->color_secondary = color_secondary;
  if (font == TIME_FONT_GARAMOND) {
    time_load_font(section, RESOURCE_ID_EB_GARAMOND);
  } else if (font == TIME_FONT_LECO) {
    time_load_font(section, RESOURCE_ID_LECO_1976);
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