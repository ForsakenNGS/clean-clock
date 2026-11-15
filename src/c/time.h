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
  char buffer[8];
  FFont *font;
  TimeConfig config;
} TimeSectionData;

typedef struct {
  Layer *layer;
  TimeSectionData *data;
} TimeSection;

TimeConfig time_config(const GColor color_primary, const GColor color_secondary, const GColor color_accent, const TimeFont font);
void time_load(TimeSection *section, Layer *root_layer, const TimeConfig config);
void time_load_settings(TimeSection *section, const TimeConfig config);
void time_load_font(TimeSection *section, const uint32_t resource_id);
void time_unload(TimeSection *section);
void time_update(TimeSection *section, struct tm *tick_time, TimeUnits units_changed);
void time_refresh(TimeSection *section);