#include "watchface.h"

static Window *s_window;
static Watchface s_watchface;

GColor watchface_color(int32_t v) {
#ifdef PBL_COLOR
  return GColorFromHEX((uint32_t)v);
#else
  return (v == 0) ? GColorBlack : GColorWhite;
#endif
}

static fixed_t fctx_scale_x(FContext *fctx, fixed_t units) {
  return units * fctx->transform_scale_to.x / fctx->transform_scale_from.x;
}

static fixed_t fctx_scale_y(FContext *fctx, fixed_t units) {
  return units * fctx->transform_scale_to.x / fctx->transform_scale_from.x;
}

static void watchface_inbox_received(DictionaryIterator *iter, void *context) {
  settings_handle_inbox(iter, context);
  watchface_refresh();
}

static void watchface_inbox_dropped(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_WARNING, "Inbox dropped: %d", (int)reason);
}

void watchface_init(void) {
  s_window = window_create();
  settings_init();
  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = watchface_load,
    .unload = watchface_unload,
  });
  window_stack_push(s_window, true);
  watchface_apply_tick_subscription();
  watchface_apply_battery_subscription();
#if defined(PBL_HEALTH)
  watchface_apply_health_subscription();
#endif
  app_message_register_inbox_received(watchface_inbox_received);
  app_message_register_inbox_dropped(watchface_inbox_dropped);
  app_message_open(512, 128);
  settings_request_weather();
}

void watchface_deinit(void) {
  tick_timer_service_unsubscribe();
  battery_state_service_unsubscribe();
#if defined(PBL_HEALTH)
  health_service_events_unsubscribe();
#endif
  window_destroy(s_window);
  settings_deinit();
}

void watchface_load(Window *window) {
  ClaySettings *s = settings_get();
  s_watchface.root_layer = window_get_root_layer(window);
#if defined(PBL_PLATFORM_EMERY)
  s_watchface.bar_font = fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK);
  s_watchface.bar_font_small = fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD);
#else
  s_watchface.bar_font = fonts_get_system_font(FONT_KEY_GOTHIC_28);
  s_watchface.bar_font_small = fonts_get_system_font(FONT_KEY_GOTHIC_14);
#endif
  bar_load(&s_watchface.top, s_watchface.root_layer, BAR_LOCATION_TOP, s->top);
  time_load(&s_watchface.time, s_watchface.root_layer, time_config(GColorWhite, GColorBlack, s->time_font));
  bar_load(&s_watchface.bottom, s_watchface.root_layer, BAR_LOCATION_BOTTOM, s->bottom);
  window_set_background_color(window, s_watchface.time.data->config.color_secondary);
  watchface_refresh();
}

void watchface_load_settings(void) {
  ClaySettings *s = settings_get();
  bar_load_settings(&s_watchface.top, s->top);
  time_load_settings(&s_watchface.time, time_config(GColorWhite, GColorBlack, s->time_font));
  bar_load_settings(&s_watchface.bottom, s->bottom);
  watchface_refresh();
}

void watchface_unload(Window *window) {
  bar_unload(&s_watchface.top);
  time_unload(&s_watchface.time);
  bar_unload(&s_watchface.bottom);
}

void watchface_refresh(void) {
  bar_refresh(&s_watchface.top);
  time_refresh(&s_watchface.time);
  bar_refresh(&s_watchface.bottom);
  layer_mark_dirty(s_watchface.root_layer);
}

static void watchface_update_battery(BatteryChargeState charge) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Battery update (%d%%)", charge.charge_percent);
  widget_update_battery(&s_watchface.top.left, charge);
  widget_update_battery(&s_watchface.top.center, charge);
  widget_update_battery(&s_watchface.top.right, charge);
  widget_update_battery(&s_watchface.bottom.left, charge);
  widget_update_battery(&s_watchface.bottom.center, charge);
  widget_update_battery(&s_watchface.bottom.right, charge);
}

static void watchface_update_health(HealthEventType event, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Health update (%d)", event);
  widget_update_health(&s_watchface.top.left, event, context);
  widget_update_health(&s_watchface.top.center, event, context);
  widget_update_health(&s_watchface.top.right, event, context);
  widget_update_health(&s_watchface.bottom.left, event, context);
  widget_update_health(&s_watchface.bottom.center, event, context);
  widget_update_health(&s_watchface.bottom.right, event, context);
}

static void watchface_update_time(struct tm *t, TimeUnits units_changed) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Time update (%d)", units_changed);
  time_update(&s_watchface.time, t, units_changed);
  widget_update_time(&s_watchface.top.left, t, units_changed);
  widget_update_time(&s_watchface.top.center, t, units_changed);
  widget_update_time(&s_watchface.top.right, t, units_changed);
  widget_update_time(&s_watchface.bottom.left, t, units_changed);
  widget_update_time(&s_watchface.bottom.center, t, units_changed);
  widget_update_time(&s_watchface.bottom.right, t, units_changed);
}

static void watchface_update_weather(void) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Weather update");
  widget_update_weather(&s_watchface.top.left);
  widget_update_weather(&s_watchface.top.center);
  widget_update_weather(&s_watchface.top.right);
  widget_update_weather(&s_watchface.bottom.left);
  widget_update_weather(&s_watchface.bottom.center);
  widget_update_weather(&s_watchface.bottom.right);
}

TimeUnits watchface_update_time_units() {
  TimeUnits units = MINUTE_UNIT | DAY_UNIT | HOUR_UNIT;
  if (watchface_has_element_of_type(ELEMENT_SECONDS)) {
    units |= SECOND_UNIT;
  }
  return units;
}

bool watchface_has_element_of_type(const ElementType type) {
  if ((s_watchface.top.left.data->config.type == type) || (s_watchface.top.center.data->config.type == type) || 
    (s_watchface.top.right.data->config.type == type) || (s_watchface.bottom.left.data->config.type == type) || 
    (s_watchface.bottom.center.data->config.type == type) || (s_watchface.bottom.right.data->config.type == type))
  {
    return true;
  } else {
    return false;
  }
}

void watchface_apply_tick_subscription(void) {
  TimeUnits desired = watchface_update_time_units();
  tick_timer_service_unsubscribe();
  tick_timer_service_subscribe(desired, watchface_update_time);
}

void watchface_apply_battery_subscription(void) {
  battery_state_service_unsubscribe();
  if (watchface_has_element_of_type(ELEMENT_BATTERY)) {
    battery_state_service_subscribe(watchface_update_battery);
  }
}

void watchface_apply_health_subscription(void) {
  battery_state_service_unsubscribe();
  if (watchface_has_element_of_type(ELEMENT_HEART_RATE)) {
    health_service_events_subscribe(watchface_update_health, NULL);
  }
}

GFont watchface_bar_font(void) {
  return s_watchface.bar_font;
}

GFont watchface_bar_font_small(void) {
  return s_watchface.bar_font_small;
}

int16_t watchface_bar_height(void) {
#if defined(PBL_PLATFORM_EMERY)
  return 48;
#else
  return 36;
#endif
}

int16_t watchface_time_height(void) {
#if defined(PBL_PLATFORM_EMERY)
  return 132;
#else
  return 96;
#endif
}