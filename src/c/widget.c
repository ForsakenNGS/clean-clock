#include "time.h"
#include "widget.h"
#include "bar.h"
#include "watchface.h"

static char widget_temp[16];
static uint32_t widget_temp_icon;

static const char *widget_weekday(int wday) {
  static const char *names[] = {"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
  return names[wday % 7];
}

static const char *widget_month(int mon) {
  static const char *names[] = {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
  return names[mon % 12];
}

static void widget_update_layer(Layer *layer, GContext *ctx) {
  WidgetData *d = (WidgetData*)layer_get_data(layer);
  GRect bounds = layer_get_bounds(layer);
  // Draw background and accents
  if (d->config.location == WIDGET_LOCATION_CENTER) {
    #ifdef PBL_COLOR
    graphics_context_set_text_color(ctx, d->config.color_primary);
    graphics_context_set_fill_color(ctx, d->config.color_accent);
    #else
    graphics_context_set_text_color(ctx, d->config.color_secondary);
    graphics_context_set_fill_color(ctx, d->config.color_primary);
    #endif
    graphics_fill_rect(ctx, bounds, 0, GCornerNone);
  } else {
    graphics_context_set_text_color(ctx, d->config.color_primary);
    graphics_context_set_fill_color(ctx, d->config.color_secondary);
    graphics_fill_rect(ctx, bounds, 0, GCornerNone);
    #ifdef PBL_COLOR
    graphics_context_set_fill_color(ctx, d->config.color_accent);
    #else
    graphics_context_set_fill_color(ctx, d->config.color_primary);
    #endif
    const int16_t accent_height = widget_get_accent_height();
    if (d->config.bar == BAR_LOCATION_TOP) {
      graphics_fill_rect(ctx, GRect(0, bounds.size.h - accent_height, bounds.size.w, accent_height), 0, GCornerNone);
    } else {
      graphics_fill_rect(ctx, GRect(0, 0, bounds.size.w, accent_height), 0, GCornerNone);
    }
  }
  // Draw text
  graphics_draw_text(ctx, d->buffer, widget_font(d->config), d->text_bounds, GTextOverflowModeTrailingEllipsis, widget_font_align(d->config), NULL);
  // Draw weather icons
  if (d->icon_pdc != NULL) {
    #if defined(PBL_PLATFORM_EMERY)
    const int16_t icon_size = 32;
    #else
    const int16_t icon_size = 32;
    #endif
    GPoint icon_pos = GPoint((bounds.size.w - icon_size) / 2, (bounds.size.h - icon_size) / 2);
    switch (d->config.location) {
      case WIDGET_LOCATION_LEFT:
        icon_pos.x = d->text_bounds.origin.x + d->text_bounds.size.w - icon_size;
        break;
      case WIDGET_LOCATION_RIGHT:
        icon_pos.x = d->text_bounds.origin.x;
        break;
      default:
        break;
    }
    gdraw_command_image_draw(ctx, d->icon_pdc, icon_pos);
  }
}

static void widget_update_text(Widget *widget) {
  if (strcmp(widget->data->buffer, widget_temp) != 0) {
    strncpy(widget->data->buffer, widget_temp, sizeof(widget->data->buffer));
    layer_mark_dirty(widget->layer);
  }
}

static void widget_update_icon(Widget *widget) {
  if (widget->data->icon != widget_temp_icon) {
    widget->data->icon = widget_temp_icon;
    if (widget->data->icon_pdc != NULL) {
      gdraw_command_image_destroy(widget->data->icon_pdc);
      widget->data->icon_pdc = NULL;
    }
    if (widget->data->icon != UINT32_MAX) {
      widget->data->icon_pdc = gdraw_command_image_create_with_resource(widget->data->icon);
    }
    layer_mark_dirty(widget->layer);
  }
}

bool widget_is_battery(Widget *widget) {
  return (widget->data->config.type == ELEMENT_BATTERY);
}

bool widget_is_health(Widget *widget) {
  return (widget->data->config.type == ELEMENT_HEART_RATE);
}

bool widget_is_time(Widget *widget) {
  switch (widget->data->config.type) {
    case ELEMENT_DAY_OF_MONTH:
    case ELEMENT_WEEKDAY:
    case ELEMENT_MONTH:
    case ELEMENT_WEEK_OF_YEAR:
    case ELEMENT_SECONDS:
      return true;
    default:
      return false;
  }
}

bool widget_is_weather(Widget *widget) {
  switch (widget->data->config.type) {
    case ELEMENT_WEATHER_CURRENT:
    case ELEMENT_WEATHER_TODAY:
    case ELEMENT_WEATHER_TODAY_MIN_MAX:
    case ELEMENT_WEATHER_TOMORROW:
    case ELEMENT_WEATHER_TOMORROW_MIN_MAX:
      return true;
    default:
      return false;
  }
}

/**
 * Get the width of the given element type if positioned in the center
 */
int16_t widget_get_center_width(ElementType type) {
#if defined(PBL_PLATFORM_EMERY)
  switch (type) {
    default:
      return 50;
  }
#else
  switch (type) {
    default:
      return 50;
  }
#endif
}

int16_t widget_get_padding_horizontal(void) {
#if defined(PBL_PLATFORM_EMERY)
  return 4;
#else
  return 2;
#endif
}

int16_t widget_get_padding_vertical(const WidgetConfig config) {
#if defined(PBL_PLATFORM_EMERY)
  if (widget_has_small_font(config)) {
      return 8;
  } else {
      return 2;
  }
#else
  if (widget_has_small_font(config)) {
      return 4;
  } else {
      return 2;
  }
#endif
}

int16_t widget_get_accent_height(void) {
#if defined(PBL_PLATFORM_EMERY)
  return 2;
#else
  return 2;
#endif
}

bool widget_has_small_font(const WidgetConfig config) {
  switch (config.type) {
    case ELEMENT_BATTERY:
    case ELEMENT_WEATHER_CURRENT:
    case ELEMENT_WEATHER_TODAY:
    case ELEMENT_WEATHER_TODAY_MIN_MAX:
    case ELEMENT_WEATHER_TOMORROW:
    case ELEMENT_WEATHER_TOMORROW_MIN_MAX:
      return true;
    default:
      return false;
  }
}

GFont widget_font(const WidgetConfig config) {
  if (widget_has_small_font(config)) {
      return watchface_bar_font_small();
  } else {
      return watchface_bar_font();
  }
}

GTextAlignment widget_font_align(const WidgetConfig config) {
  switch (config.location) {
    default:
    case WIDGET_LOCATION_LEFT:
      return GTextAlignmentLeft;
    case WIDGET_LOCATION_CENTER:
      return GTextAlignmentCenter;
    case WIDGET_LOCATION_RIGHT:
      return GTextAlignmentRight;
  }
}

uint32_t widget_weather_icon(int condition) {
  switch (condition) {
    case 0: // Clear day
    case 1: // Clear night
      #if defined(PBL_PLATFORM_EMERY)
      return RESOURCE_ID_SUN_32;
      #else
      return RESOURCE_ID_SUN_24;
      #endif
    case 2: // Cloudy day      
      #if defined(PBL_PLATFORM_EMERY)
      return RESOURCE_ID_CLOUD_32;
      #else
      return RESOURCE_ID_CLOUD_24;
      #endif
    case 3: // Heavy rain
    case 5: // Light rain
      #if defined(PBL_PLATFORM_EMERY)
      return RESOURCE_ID_RAIN_32;
      #else
      return RESOURCE_ID_RAIN_24;
      #endif
    case 4: // Heavy snow
    case 6: // Light snow
    case 9: // Raining and snowing
      #if defined(PBL_PLATFORM_EMERY)
      return RESOURCE_ID_SNOW_32;
      #else
      return RESOURCE_ID_SNOW_24;
      #endif
    default:
    case 7: // Partly cloudy night
    case 8: // Partly cloudy day
      #if defined(PBL_PLATFORM_EMERY)
      return RESOURCE_ID_SUN_CLOUD_32;
      #else
      return RESOURCE_ID_SUN_CLOUD_24;
      #endif
    case 10: // Thunderstorm
      #if defined(PBL_PLATFORM_EMERY)
      return RESOURCE_ID_STORM_32;
      #else
      return RESOURCE_ID_STORM_24;
      #endif
  }
}

WidgetConfig widget_config(const BarLocation bar, const WidgetLocation location, const ElementType type, const GColor color_primary, const GColor color_secondary, const GColor color_accent) {
  WidgetConfig config;
  config.bar = bar;
  config.location = location;
  config.type = type;
  config.color_primary = color_primary;
  config.color_secondary = color_secondary;
  config.color_accent = color_accent;
  return config;
}

void widget_load(Widget *widget, Layer *bar_layer, const WidgetConfig config, const int16_t width) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Widget type: %d", config.type);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Widget width: %d", width);
  GRect bounds = layer_get_bounds(bar_layer);
  // Create widget segment
  int16_t left = 0;
  if (config.location == WIDGET_LOCATION_CENTER) {
    left = (bounds.size.w - width) / 2;
  } else if (config.location == WIDGET_LOCATION_RIGHT) {
    left = bounds.size.w - width;
  }
  const GRect widget_bounds = GRect(left, 0, width, watchface_bar_height());
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Widget rect: (%d, %d, %d, %d)", widget_bounds.origin.x, widget_bounds.origin.y, widget_bounds.size.w, widget_bounds.size.h);
  widget->layer = layer_create_with_data(
    widget_bounds,
    sizeof(WidgetData)
  );
  widget->data = (WidgetData*)layer_get_data(widget->layer);
  const int16_t px = widget_get_padding_horizontal();
  const int16_t py = widget_get_padding_vertical(config);
  widget->data->text_bounds = GRect(px, py, widget_bounds.size.w - px * 2, widget_bounds.size.h - py * 2);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Widget text rect: (%d, %d, %d, %d)", widget->data->text_bounds.origin.x, widget->data->text_bounds.origin.y, widget->data->text_bounds.size.w, widget->data->text_bounds.size.h);
  widget_load_settings(widget, config);
  layer_set_update_proc(widget->layer, widget_update_layer);
  layer_add_child(bar_layer, widget->layer);
}

void widget_load_settings(Widget *widget, const WidgetConfig config) {
  widget->data->icon = UINT32_MAX; // UINT32_MAX -> no icon
  widget->data->icon_pdc = NULL;
  widget->data->config = config;
  GRect widget_bounds = layer_get_bounds(widget->layer);
  int16_t px = widget_get_padding_horizontal();
  int16_t py = widget_get_padding_vertical(config);
  widget->data->text_bounds = GRect(px, py, widget_bounds.size.w - px * 2, widget_bounds.size.h - py * 2);
}

void widget_unload(Widget *widget) {
  if (widget->data->icon_pdc != NULL) {
    gdraw_command_image_destroy(widget->data->icon_pdc);
    widget->data->icon_pdc = NULL;
  }
  layer_destroy(widget->layer);
}

void widget_update_any(Widget *widget) {
  time_t time_now;
  struct tm* time_now_local;
  TimeUnits units_changed = SECOND_UNIT || MINUTE_UNIT || HOUR_UNIT || DAY_UNIT || MONTH_UNIT || YEAR_UNIT;
  time(&time_now);
  time_now_local = localtime(&time_now);
  BatteryChargeState charge = battery_state_service_peek();
  widget_update_battery(widget, charge);
  widget_update_health(widget, HealthEventHeartRateUpdate, NULL);
  widget_update_weather(widget);
  widget_update_time(widget, time_now_local, units_changed);
}

void widget_update_battery(Widget *widget, BatteryChargeState charge) {
  if (!widget_is_battery(widget)) {
    return;
  }
  switch (widget->data->config.type) {
    case ELEMENT_BATTERY:
      snprintf(widget_temp, sizeof(widget_temp), "%d", charge.charge_percent);
      break;
    default:
      return;
  }
  widget_update_text(widget);
}

void widget_update_health(Widget *widget, HealthEventType event, void *context) {
  if (!widget_is_health(widget)) {
    return;
  }
  switch (widget->data->config.type) {
    case ELEMENT_HEART_RATE:
#if defined(PBL_HEALTH)
      {
        HealthValue hr = health_service_peek_current_value(HealthMetricHeartRateBPM);
        if (hr > 0) snprintf(widget_temp, sizeof(widget_temp), "%d", (int)hr);
        else snprintf(widget_temp, sizeof(widget_temp), "--");
      }
#else
      snprintf(widget_temp, sizeof(widget_temp), "--");
#endif
    default:
      return;
  }
  widget_update_text(widget);
}

void widget_update_weather(Widget *widget) {
  if (!widget_is_weather(widget)) {
    return;
  }
  WeatherData *w = weather_get();
  widget_temp_icon = UINT32_MAX;
  switch (widget->data->config.type) {
    case ELEMENT_WEATHER_CURRENT:
      if (w->currentCondition > -1) {
        widget_temp_icon = widget_weather_icon(w->currentCondition);
        snprintf(widget_temp, sizeof(widget_temp), "%d°", (int)w->currentTemp);
      } else {
        snprintf(widget_temp, sizeof(widget_temp), "--°");
      }
      break;
    case ELEMENT_WEATHER_TODAY:
      if (w->todaysCondition > -1) {
        widget_temp_icon = widget_weather_icon(w->todaysCondition);
        snprintf(widget_temp, sizeof(widget_temp), "%d°", (int)w->todaysMaxTemp);
      } else {
        snprintf(widget_temp, sizeof(widget_temp), "--°");
      }
      break;
    case ELEMENT_WEATHER_TODAY_MIN_MAX:
      if (w->todaysCondition > -1) {
        snprintf(widget_temp, sizeof(widget_temp), "%d/%d°", (int)w->tomorrowsMinTemp, (int)w->tomorrowsMaxTemp);
      } else {
        snprintf(widget_temp, sizeof(widget_temp), "--/--");
      }
      break;
    case ELEMENT_WEATHER_TOMORROW:
      if (w->tomorrowsCondition > -1) {
        widget_temp_icon = widget_weather_icon(w->tomorrowsCondition);
        snprintf(widget_temp, sizeof(widget_temp), "%d°", (int)w->tomorrowsMaxTemp);
      } else {
        snprintf(widget_temp, sizeof(widget_temp), "--°");
      }
      break;
    case ELEMENT_WEATHER_TOMORROW_MIN_MAX:
      if (w->tomorrowsCondition > -1) {
        snprintf(widget_temp, sizeof(widget_temp), "%d/%d°", (int)w->todaysMinTemp, (int)w->todaysMaxTemp);
      } else {
        snprintf(widget_temp, sizeof(widget_temp), "--/--");
      }
      break;
    default:
      return;
  }
  widget_update_text(widget);
  widget_update_icon(widget);
}

void widget_update_time(Widget *widget, struct tm *t, TimeUnits units_changed) {
  if (!widget_is_time(widget)) {
    return;
  }
  switch (widget->data->config.type) {
    case ELEMENT_DAY_OF_MONTH:
      if (units_changed || DAY_UNIT == 0) {
        return;
      }
      snprintf(widget_temp, sizeof(widget_temp), "%d", t->tm_mday);
      break;
    case ELEMENT_WEEKDAY:
      if (units_changed || DAY_UNIT == 0) {
        return;
      }
      snprintf(widget_temp, sizeof(widget_temp), "%s", widget_weekday(t->tm_wday));
      break;
    case ELEMENT_MONTH:
      if (units_changed || MONTH_UNIT == 0) {
        return;
      }
      snprintf(widget_temp, sizeof(widget_temp), "%s", widget_month(t->tm_mon));
      break;
    case ELEMENT_WEEK_OF_YEAR:
      if (units_changed || DAY_UNIT == 0) {
        return;
      }
      strftime(widget_temp, sizeof(widget_temp), "%V", t);
      break;
    case ELEMENT_SECONDS:
      snprintf(widget_temp, sizeof(widget_temp), "%02d", t->tm_sec);
      break;
    default:
      return;
  }
  widget_update_text(widget);
}

void widget_refresh(Widget *widget) {
  widget_update_any(widget);
  layer_mark_dirty(widget->layer);
}