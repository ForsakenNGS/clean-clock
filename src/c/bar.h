#pragma once
#include <pebble.h>
#include "widget.h"

typedef struct {
  Layer *layer;
  BarLocation location;
  Widget left;
  Widget center;
  Widget right;
} BarSection;

void bar_load(BarSection *section, Layer *root_layer, const BarLocation location, const BarSettings bs);
void bar_load_settings(BarSection *section, const BarSettings bs);
void bar_unload(BarSection *section);
void bar_refresh(BarSection *section);
int16_t bar_get_widget_width(const BarSettings bs, const WidgetLocation location, const GRect bounds);