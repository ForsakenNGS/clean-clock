#pragma once
#include <pebble.h>
#include "settings.h"

typedef enum {
  WIDGET_LOCATION_LEFT = 0,
  WIDGET_LOCATION_CENTER = 1,
  WIDGET_LOCATION_RIGHT = 2
} WidgetLocation;

typedef struct {
  TextLayer *layer;
  ElementType type;
  WidgetLocation location;
  GColor color_primary;
  GColor color_secondary;
  char buffer[16];
} Widget;

bool widget_is_battery(Widget *widget);
bool widget_is_health(Widget *widget);
bool widget_is_time(Widget *widget);
bool widget_is_weather(Widget *widget);

int16_t widget_get_center_width(ElementType type);
void widget_load(Widget *widget, Layer *bar_layer, const WidgetLocation location, const ElementType type, const GColor color_primary, const GColor color_secondary, const int16_t width);
void widget_load_settings(Widget *widget, const ElementType type, const WidgetLocation location, const GColor color_primary, const GColor color_secondary);
void widget_unload(Widget *widget);
void widget_update_any(Widget *widget);
void widget_update_battery(Widget *widget, BatteryChargeState charge);
void widget_update_health(Widget *widget, HealthEventType event, void *context);
void widget_update_time(Widget *widget, struct tm *t, TimeUnits units_changed);
void widget_update_weather(Widget *widget);
void widget_refresh(Widget *widget);