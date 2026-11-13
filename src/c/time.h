#pragma once
#include <pebble.h>
#include "fctx/fctx.h"
#include "fctx/ffont.h"
#include "settings.h"

typedef struct {
  char buffer[8];
  GColor color_primary;
  GColor color_secondary;
  FFont *font;
} TimeSectionData;

typedef struct {
  Layer *layer;
  TimeSectionData *data;
} TimeSection;

void time_load(TimeSection *section, Layer *root_layer);
void time_load_settings(TimeSection *section, const GColor color_primary, const GColor color_secondary, const TimeFont font);
void time_load_font(TimeSection *section, const uint32_t resource_id);
void time_unload(TimeSection *section);
void time_update(TimeSection *section, struct tm *tick_time, TimeUnits units_changed);
void time_refresh(TimeSection *section);