#pragma once
#include <pebble.h>
#include "fctx/fctx.h"
#include "fctx/ffont.h"
#include "settings.h"

typedef struct {
  GColor color_primary;
  GColor color_secondary;
  GColor color_accent;
  TimeFont font;
} TimeConfig;

typedef struct {
  bool redraw_due;
  GRect bounds;
  char buffer[8];
  FFont *font;
  TimeConfig config;
} TimeSection;

TimeConfig time_config(const GColor color_primary, const GColor color_secondary, const GColor color_accent, const TimeFont font);
GRect time_rect(TimeSection *section, int16_t x, int16_t y, int16_t w, int16_t h);
void time_load(TimeSection *section, Layer *root_layer, const TimeConfig config);
void time_load_settings(TimeSection *section, const TimeConfig config);
void time_load_font(TimeSection *section, const uint32_t resource_id);
void time_unload(TimeSection *section);
void time_update(TimeSection *section, struct tm *tick_time, TimeUnits units_changed);
void time_update_layer(TimeSection *section, Layer *layer, GContext *ctx);
void time_refresh(TimeSection *section);