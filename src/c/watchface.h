#pragma once
#include "bar.h"
#include "time.h"

typedef struct {
  Layer *root_layer;
  FFont *bar_ffont;
  TimeSection time;
  BarSection top;
  BarSection bottom;
} Watchface;

GColor watchface_color(int32_t v);
void watchface_init(void);
void watchface_deinit(void);
void watchface_load(Window *window);
void watchface_load_settings(void);
void watchface_unload(Window *window);
void watchface_refresh(void);
void watchface_redraw(void);
TimeUnits watchface_update_time_units();
void watchface_update_weather(void);
bool watchface_has_element_of_type(const ElementType type);
void watchface_apply_subscriptions(void);
void watchface_apply_tick_subscription(void);
void watchface_apply_battery_subscription(void);
void watchface_apply_health_subscription(void);
FFont* watchface_bar_ffont(void);
int16_t watchface_bar_height(void);
int16_t watchface_accent_height(void);
int16_t watchface_time_height(void);