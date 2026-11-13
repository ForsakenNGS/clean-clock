#include "time.h"
#include "watchface.h"

static char time_temp[8];

static void time_update_layer(Layer *layer, GContext *ctx) {
  TimeSectionData *d = (TimeSectionData*)layer_get_data(layer);
  if (!d->font) return;

  GRect bounds = layer_get_bounds(layer);
  int16_t font_size = watchface_time_font_size();
  int16_t font_offset = watchface_time_font_offset();
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);
  
  FContext fctx;
  fctx_init_context(&fctx, ctx);
  fctx_set_fill_color(&fctx, GColorWhite);
  fctx_set_text_em_height(&fctx, d->font, font_size);
  fctx_set_offset(&fctx, FPointI(bounds.size.w / 2, font_offset));
  fctx_begin_fill(&fctx);
  fctx_draw_string(&fctx, d->buffer, d->font, GTextAlignmentCenter, FTextAnchorTop);
  fctx_end_fill(&fctx);
  fctx_deinit_context(&fctx);
}

void time_load(TimeSection *section, Layer *root_layer) {
  GRect bounds = layer_get_bounds(root_layer);
  ClaySettings *s = settings_get();
  // Create time segment
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Time rect: (%d, %d, %d, %d)", 0, watchface_bar_height(), bounds.size.w, watchface_time_height());
  section->layer = layer_create_with_data(
    GRect(0, watchface_bar_height(), bounds.size.w, watchface_time_height()),
    sizeof(TimeSectionData)
  );
  section->data = (TimeSectionData*)layer_get_data(section->layer);
  time_load_settings(section, GColorBlack, GColorWhite, s->time_font);
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