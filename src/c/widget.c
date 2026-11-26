#include "time.h"
#include "widget.h"
#include "bar.h"
#include "text.h"
#include "watchface.h"

static char widget_temp[16];
static uint32_t widget_temp_icon;

static const char *widget_weekday(int wday) {
  static const char *names[] = {"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
  return names[wday % 7];
}

// Convert a temperature stored in Celsius to the configured unit.
static int widget_temp_to_unit(int temp_c) {
  ClaySettings *s = settings_get();
  if (s->temp_unit == TEMP_UNIT_F) {
    return (int)(temp_c * 9.0 / 5.0 + 32.0);
  }
  return temp_c;
}

static const char *widget_month(int mon) {
  static const char *names[] = {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
  return names[mon % 12];
}

static void widget_update_text(Widget *widget) {
  if (strcmp(widget->buffer, widget_temp) != 0) {
    strncpy(widget->buffer, widget_temp, sizeof(widget->buffer));
    widget_redraw(widget);
  }
}

static void widget_update_icon(Widget *widget) {
  if (widget->icon != widget_temp_icon) {
    widget->icon = widget_temp_icon;
    if (widget->icon_pdc != NULL) {
      gdraw_command_image_destroy(widget->icon_pdc);
      widget->icon_pdc = NULL;
    }
    if (widget->icon != UINT32_MAX) {
      widget->icon_pdc = gdraw_command_image_create_with_resource(widget->icon);
    }
    widget_redraw(widget);
  }
}

bool widget_is_battery(Widget *widget) {
  return (widget->config.type == ELEMENT_BATTERY);
}

bool widget_is_health(Widget *widget) {
  return (widget->config.type == ELEMENT_HEART_RATE);
}

bool widget_is_time(Widget *widget) {
  switch (widget->config.type) {
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
  switch (widget->config.type) {
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

bool widget_has_icon(Widget *widget) {
  switch (widget->config.type) {
    case ELEMENT_WEATHER_CURRENT:
    case ELEMENT_WEATHER_TODAY:
    case ELEMENT_WEATHER_TOMORROW:
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
      return 40;
  }
#endif
}

int16_t widget_get_icon_size(void) {
#if defined(PBL_PLATFORM_EMERY)
  return 32;
#elif defined(PBL_PLATFORM_APLITE)
  return 16;
#else
  return 24;
#endif
}

int16_t widget_get_padding_horizontal(void) {
#if defined(PBL_PLATFORM_EMERY)
  return 4;
#elif defined(PBL_PLATFORM_BASALT)
  return 4;
#else
  return 2;
#endif
}

int16_t widget_get_padding_vertical(const WidgetConfig config) {
#if defined(PBL_PLATFORM_EMERY)
  return 6;
#elif defined(PBL_PLATFORM_BASALT)
  return 6;
#elif defined(PBL_PLATFORM_APLITE)
  return 5;
#else
  return 6;
#endif
}

GRect widget_get_text_bounds(Widget *widget) {
  const GRect widget_bounds = widget->bounds;
  int16_t px = widget_get_padding_horizontal();
  int16_t py = widget_get_padding_vertical(widget->config);
  if (widget_has_icon(widget)) {
    const int16_t icon_size = widget_get_icon_size();
    switch (widget->config.location) {
      default:
      case WIDGET_LOCATION_LEFT:
        return widget_rect(widget, px, py, widget_bounds.size.w - px * 2 - icon_size, widget_bounds.size.h - py * 2);
      case WIDGET_LOCATION_RIGHT:
        return widget_rect(widget, px + icon_size, py, widget_bounds.size.w - px * 2 - icon_size, widget_bounds.size.h - py * 2);
    }
  } else {
    return widget_rect(widget, px, py, widget_bounds.size.w - px * 2, widget_bounds.size.h - py * 2);
  }
}

uint32_t widget_weather_icon(int condition) {
  return widget_weather_icon_for_color(condition, GColorWhite);
}

// Select the matching color variant of a weather icon. The icon palette only
// contains black and white strokes, so the variant is chosen based on the
// configured primary color: a light primary color renders the white icon, a
// dark primary color renders the black icon.
uint32_t widget_weather_icon_for_color(int condition, GColor color_primary) {
  // GColorBlack has alpha 0b11 and RGB all 0; GColorWhite has alpha 0b11 and RGB
  // all 1. Compare the RGB portion to decide whether the primary color is dark.
  bool is_dark = (color_primary.argb & 0x3F) == 0;
  switch (condition) {
    case 0: // Clear day
    case 1: // Clear night
      #if defined(PBL_PLATFORM_EMERY)
      return is_dark ? RESOURCE_ID_SUN_BLACK_32 : RESOURCE_ID_SUN_WHITE_32;
      #elif defined(PBL_PLATFORM_APLITE)
      return is_dark ? RESOURCE_ID_SUN_BLACK_16 : RESOURCE_ID_SUN_WHITE_16;
      #else
      return is_dark ? RESOURCE_ID_SUN_BLACK_24 : RESOURCE_ID_SUN_WHITE_24;
      #endif
    case 2: // Cloudy day
      #if defined(PBL_PLATFORM_EMERY)
      return is_dark ? RESOURCE_ID_CLOUD_BLACK_32 : RESOURCE_ID_CLOUD_WHITE_32;
      #elif defined(PBL_PLATFORM_APLITE)
      return is_dark ? RESOURCE_ID_CLOUD_BLACK_16 : RESOURCE_ID_CLOUD_WHITE_16;
      #else
      return is_dark ? RESOURCE_ID_CLOUD_BLACK_24 : RESOURCE_ID_CLOUD_WHITE_24;
      #endif
    case 3: // Heavy rain
    case 5: // Light rain
      #if defined(PBL_PLATFORM_EMERY)
      return is_dark ? RESOURCE_ID_RAIN_BLACK_32 : RESOURCE_ID_RAIN_WHITE_32;
      #elif defined(PBL_PLATFORM_APLITE)
      return is_dark ? RESOURCE_ID_RAIN_BLACK_16 : RESOURCE_ID_RAIN_WHITE_16;
      #else
      return is_dark ? RESOURCE_ID_RAIN_BLACK_24 : RESOURCE_ID_RAIN_WHITE_24;
      #endif
    case 4: // Heavy snow
    case 6: // Light snow
    case 9: // Raining and snowing
      #if defined(PBL_PLATFORM_EMERY)
      return is_dark ? RESOURCE_ID_SNOW_BLACK_32 : RESOURCE_ID_SNOW_WHITE_32;
      #elif defined(PBL_PLATFORM_APLITE)
      return is_dark ? RESOURCE_ID_SNOW_BLACK_16 : RESOURCE_ID_SNOW_WHITE_16;
      #else
      return is_dark ? RESOURCE_ID_SNOW_BLACK_24 : RESOURCE_ID_SNOW_WHITE_24;
      #endif
    default:
    case 7: // Partly cloudy night
    case 8: // Partly cloudy day
      #if defined(PBL_PLATFORM_EMERY)
      return is_dark ? RESOURCE_ID_SUN_CLOUD_BLACK_32 : RESOURCE_ID_SUN_CLOUD_WHITE_32;
      #elif defined(PBL_PLATFORM_APLITE)
      return is_dark ? RESOURCE_ID_SUN_CLOUD_BLACK_16 : RESOURCE_ID_SUN_CLOUD_WHITE_16;
      #else
      return is_dark ? RESOURCE_ID_SUN_CLOUD_BLACK_24 : RESOURCE_ID_SUN_CLOUD_WHITE_24;
      #endif
    case 10: // Thunderstorm
      #if defined(PBL_PLATFORM_EMERY)
      return is_dark ? RESOURCE_ID_STORM_BLACK_32 : RESOURCE_ID_STORM_WHITE_32;
      #elif defined(PBL_PLATFORM_APLITE)
      return is_dark ? RESOURCE_ID_STORM_BLACK_16 : RESOURCE_ID_STORM_WHITE_16;
      #else
      return is_dark ? RESOURCE_ID_STORM_BLACK_24 : RESOURCE_ID_STORM_WHITE_24;
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

GRect widget_rect(Widget *widget, int16_t x, int16_t y, int16_t w, int16_t h) {
  return GRect(
    widget->bounds.origin.x + x,
    widget->bounds.origin.y + y,
    w, h
  );
}

void widget_load(Widget *widget, const GRect bounds, const WidgetConfig config, const int16_t width) {
  // Create widget segment
  int16_t left = 0;
  if (config.location == WIDGET_LOCATION_CENTER) {
    left = (bounds.size.w - width) / 2;
  } else if (config.location == WIDGET_LOCATION_RIGHT) {
    left = bounds.size.w - width;
  }
  widget->bounds = GRect(bounds.origin.x + left, bounds.origin.y, width, watchface_bar_height());
  widget_load_settings(widget, bounds, config, width);
}

void widget_load_settings(Widget *widget, const GRect bounds, const WidgetConfig config, const int16_t width) {
  //APP_LOG(APP_LOG_LEVEL_DEBUG, "Widget type: %d", config.type);
  //APP_LOG(APP_LOG_LEVEL_DEBUG, "Widget width: %d", width);
  // Update config
  widget->icon = UINT32_MAX; // UINT32_MAX -> no icon
  widget->icon_pdc = NULL;
  widget->config = config;
  // Recalculate widget bounds
  int16_t left = 0;
  if (config.location == WIDGET_LOCATION_CENTER) {
    left = (bounds.size.w - width) / 2;
  } else if (config.location == WIDGET_LOCATION_RIGHT) {
    left = bounds.size.w - width;
  }
  widget->bounds = GRect(bounds.origin.x + left, bounds.origin.y, width, watchface_bar_height());
  //APP_LOG(APP_LOG_LEVEL_DEBUG, "Widget rect: (%d, %d, %d, %d)", widget->bounds.origin.x, widget->bounds.origin.y, widget->bounds.size.w, widget->bounds.size.h);
  // Recalculate text bounds
  widget->text_bounds = widget_get_text_bounds(widget);
  //APP_LOG(APP_LOG_LEVEL_DEBUG, "Widget text rect: (%d, %d, %d, %d)", widget->text_bounds.origin.x, widget->text_bounds.origin.y, widget->text_bounds.size.w, widget->text_bounds.size.h);
}

void widget_unload(Widget *widget) {
  if (widget->icon_pdc != NULL) {
    gdraw_command_image_destroy(widget->icon_pdc);
    widget->icon_pdc = NULL;
  }
}

void widget_update_layer(Widget* widget, Layer *layer, GContext *ctx) {
  if (widget->config.type == ELEMENT_NONE) {
    return;
  }
  if (!widget->redraw_due) {
    return;
  }
  //APP_LOG(APP_LOG_LEVEL_DEBUG, "widget_update_layer: %d, %d", widget->config.bar, widget->config.location);
  widget->redraw_due = false;
  GColor text_color = widget->config.color_primary;
  // Draw background and accents
  if (widget->config.location == WIDGET_LOCATION_CENTER) {
    #ifdef PBL_COLOR
    graphics_context_set_fill_color(ctx, widget->config.color_accent);
    #else
    text_color = widget->config.color_secondary;
    graphics_context_set_fill_color(ctx, widget->config.color_primary);
    #endif
    graphics_fill_rect(ctx, widget->bounds, 0, GCornerNone);
  } else {
    graphics_context_set_fill_color(ctx, widget->config.color_secondary);
    graphics_fill_rect(ctx, widget->bounds, 0, GCornerNone);
    #ifdef PBL_COLOR
    graphics_context_set_fill_color(ctx, widget->config.color_accent);
    #else
    graphics_context_set_fill_color(ctx, widget->config.color_primary);
    #endif
    const int16_t accent_height = watchface_accent_height();
    if (widget->config.bar == BAR_LOCATION_TOP) {
      graphics_fill_rect(ctx, widget_rect(widget, 0, widget->bounds.size.h - accent_height, widget->bounds.size.w, accent_height), 0, GCornerNone);
    } else {
      graphics_fill_rect(ctx, widget_rect(widget, 0, 0, widget->bounds.size.w, accent_height), 0, GCornerNone);
    }
  }
  // Draw text
  FFont* font = watchface_bar_ffont();
  if (font != NULL) {
    GRect text_bounds = layer_convert_rect_to_screen(layer, widget->text_bounds);
    text_render_fit(ctx, text_bounds, widget->buffer, font, text_color, GTextAlignmentCenter);
  } else {
    // Fallback if loading font fails (low memory devices)
    const GFont font = fonts_get_system_font(FONT_KEY_ROBOTO_CONDENSED_21);
    graphics_context_set_text_color(ctx, text_color);
    graphics_draw_text(ctx, widget->buffer, font, widget->text_bounds, GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
  }
  // Draw weather icons
  if (widget->icon_pdc != NULL) {
    const int16_t icon_size = widget_get_icon_size();
    GPoint icon_pos = GPoint(
      widget->bounds.origin.x + (widget->bounds.size.w - icon_size) / 2, 
      widget->bounds.origin.y + (widget->bounds.size.h - icon_size) / 2
    );
    switch (widget->config.location) {
      case WIDGET_LOCATION_LEFT:
        icon_pos.x = widget->text_bounds.origin.x + widget->text_bounds.size.w;
        break;
      case WIDGET_LOCATION_RIGHT:
        icon_pos.x = widget->text_bounds.origin.x - icon_size;
        break;
      default:
        break;
    }
    gdraw_command_image_draw(ctx, widget->icon_pdc, icon_pos);
  }
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
  switch (widget->config.type) {
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
  switch (widget->config.type) {
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
      break;
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
  switch (widget->config.type) {
    case ELEMENT_WEATHER_CURRENT:
      if (w->currentCondition > -1) {
        widget_temp_icon = widget_weather_icon_for_color(w->currentCondition, widget->config.color_primary);
        snprintf(widget_temp, sizeof(widget_temp), "%d°", widget_temp_to_unit(w->currentTemp));
      } else {
        snprintf(widget_temp, sizeof(widget_temp), "--°");
      }
      break;
    case ELEMENT_WEATHER_TODAY:
      if (w->todaysCondition > -1) {
        widget_temp_icon = widget_weather_icon_for_color(w->todaysCondition, widget->config.color_primary);
        snprintf(widget_temp, sizeof(widget_temp), "%d°", widget_temp_to_unit(w->todaysMaxTemp));
      } else {
        snprintf(widget_temp, sizeof(widget_temp), "--°");
      }
      break;
    case ELEMENT_WEATHER_TODAY_MIN_MAX:
      if (w->todaysCondition > -1) {
        snprintf(widget_temp, sizeof(widget_temp), "%d/%d°", widget_temp_to_unit(w->tomorrowsMinTemp), widget_temp_to_unit(w->tomorrowsMaxTemp));
      } else {
        snprintf(widget_temp, sizeof(widget_temp), "--/--");
      }
      break;
    case ELEMENT_WEATHER_TOMORROW:
      if (w->tomorrowsCondition > -1) {
        widget_temp_icon = widget_weather_icon_for_color(w->tomorrowsCondition, widget->config.color_primary);
        snprintf(widget_temp, sizeof(widget_temp), "%d°", widget_temp_to_unit(w->tomorrowsMaxTemp));
      } else {
        snprintf(widget_temp, sizeof(widget_temp), "--°");
      }
      break;
    case ELEMENT_WEATHER_TOMORROW_MIN_MAX:
      if (w->tomorrowsCondition > -1) {
        snprintf(widget_temp, sizeof(widget_temp), "%d/%d°", widget_temp_to_unit(w->todaysMinTemp), widget_temp_to_unit(w->todaysMaxTemp));
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
  switch (widget->config.type) {
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
  widget_redraw(widget);
}

void widget_redraw(Widget *widget) {
  widget->redraw_due = true;
  watchface_redraw();
}