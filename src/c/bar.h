#pragma once
#include <pebble.h>
#include "widget.h"

typedef enum {
  BAR_LOCATION_TOP = 0,
  BAR_LOCATION_BOTTOM = 1
} BarLocation;

typedef struct {
  Layer *layer;
  Widget left;
  Widget center;
  Widget right;
} BarSection;

void bar_load(BarSection *section, Layer *root_layer, const BarLocation location);
void bar_load_settings(BarSection *section, const ElementType left, const ElementType center, const ElementType right, const GColor color_primary, const GColor color_secondary);
void bar_unload(BarSection *section);
void bar_refresh(BarSection *section);
int16_t bar_get_widget_width(BarSection *section, const WidgetLocation location, const GRect bounds);