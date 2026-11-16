#pragma once
#include <pebble.h>
#include "settings.h"

typedef enum {
  BAR_LOCATION_TOP = 0,
  BAR_LOCATION_BOTTOM = 1
} BarLocation;

typedef enum {
  WIDGET_LOCATION_LEFT = 0,
  WIDGET_LOCATION_CENTER = 1,
  WIDGET_LOCATION_RIGHT = 2
} WidgetLocation;

typedef struct {
  BarLocation bar;
  WidgetLocation location;
  ElementType type;
  GColor color_primary;
  GColor color_secondary;
  GColor color_accent;
} WidgetConfig;

typedef struct {
  GRect text_bounds;
  WidgetConfig config;
  char buffer[16];
  uint32_t icon;
  GDrawCommandImage* icon_pdc;
} WidgetData;

typedef struct {
  Layer *layer;
  WidgetData *data;
} Widget;

bool widget_is_battery(Widget *widget);
bool widget_is_health(Widget *widget);
bool widget_is_time(Widget *widget);
bool widget_is_weather(Widget *widget);
bool widget_has_icon(Widget *widget);
int16_t widget_get_center_width(ElementType type);
int16_t widget_get_icon_size(void);
int16_t widget_get_padding_horizontal(void);
int16_t widget_get_padding_vertical(const WidgetConfig config);
GRect widget_get_text_bounds(Widget *widget);
uint32_t widget_weather_icon(int condition);
uint32_t widget_weather_icon_for_color(int condition, GColor color_primary);

WidgetConfig widget_config(const BarLocation bar, const WidgetLocation location, const ElementType type, const GColor color_primary, const GColor color_secondary, const GColor color_accent);
void widget_load(Widget *widget, Layer *bar_layer, const WidgetConfig config, const int16_t width);
void widget_load_settings(Widget *widget, Layer *bar_layer, const WidgetConfig config, const int16_t width);
void widget_unload(Widget *widget);
void widget_update_any(Widget *widget);
void widget_update_battery(Widget *widget, BatteryChargeState charge);
void widget_update_health(Widget *widget, HealthEventType event, void *context);
void widget_update_time(Widget *widget, struct tm *t, TimeUnits units_changed);
void widget_update_weather(Widget *widget);
void widget_refresh(Widget *widget);