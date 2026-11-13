#include "time.h"
#include "widget.h"
#include "bar.h"
#include "watchface.h"

static char widget_temp[16];

static const char *widget_weekday(int wday) {
  static const char *names[] = {"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
  return names[wday % 7];
}

static const char *widget_month(int mon) {
  static const char *names[] = {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
  return names[mon % 12];
}

static void widget_update_layer(Widget *widget) {
  if (strcmp(widget->buffer, widget_temp) != 0) {
    strncpy(widget->buffer, widget_temp, sizeof(widget->buffer));
    text_layer_set_text(widget->layer, widget->buffer);
  }
}

bool widget_is_battery(Widget *widget) {
  return (widget->type == ELEMENT_BATTERY);
}

bool widget_is_health(Widget *widget) {
  return (widget->type == ELEMENT_HEART_RATE);
}

bool widget_is_time(Widget *widget) {
  switch (widget->type) {
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
  switch (widget->type) {
    case ELEMENT_WEATHER_CURRENT:
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
    case ELEMENT_BATTERY:
      return 72;
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

void widget_load(Widget *widget, Layer *bar_layer, const WidgetLocation location, const ElementType type, const GColor color_primary, const GColor color_secondary, const int16_t width) {
  GRect bounds = layer_get_bounds(bar_layer);
  // Create widget segment
  int16_t left = 0;
  if (location == WIDGET_LOCATION_CENTER) {
    left = (bounds.size.w - width) / 2;
  } else if (location == WIDGET_LOCATION_RIGHT) {
    left = bounds.size.w - width;
  }
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Widget type: %d", type);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Widget width: %d", width);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Widget rect: (%d, %d, %d, %d)", left, 0, width, watchface_bar_height());
  widget->layer = text_layer_create(
    GRect(left, 0, width, watchface_bar_height())
  );
  widget_load_settings(widget, type, location, color_primary, color_secondary);
  layer_add_child(bar_layer, text_layer_get_layer(widget->layer));
}

void widget_load_settings(Widget *widget, const ElementType type, const WidgetLocation location, const GColor color_primary, const GColor color_secondary) {
  widget->type = type;
  widget->location = location;
  widget->color_primary = color_primary;
  widget->color_secondary = color_secondary;
  switch (location) {
    default:
    case WIDGET_LOCATION_LEFT:
      text_layer_set_text_color(widget->layer, color_primary);
      text_layer_set_background_color(widget->layer, color_secondary);
      text_layer_set_text_alignment(widget->layer, GTextAlignmentLeft);
      break;
    case WIDGET_LOCATION_CENTER:
      text_layer_set_text_color(widget->layer, color_secondary);
      text_layer_set_background_color(widget->layer, color_primary);
      text_layer_set_text_alignment(widget->layer, GTextAlignmentCenter);
      break;
    case WIDGET_LOCATION_RIGHT:
      text_layer_set_text_color(widget->layer, color_primary);
      text_layer_set_background_color(widget->layer, color_secondary);
      text_layer_set_text_alignment(widget->layer, GTextAlignmentRight);
      break;
  }
  text_layer_set_font(widget->layer, watchface_bar_font());
  text_layer_set_overflow_mode(widget->layer, GTextOverflowModeTrailingEllipsis);
}

void widget_unload(Widget *widget) {
  text_layer_destroy(widget->layer);
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
  switch (widget->type) {
    case ELEMENT_BATTERY:
      snprintf(widget_temp, sizeof(widget_temp), "%d", charge.charge_percent);
      break;
    default:
      return;
  }
  widget_update_layer(widget);
}

void widget_update_health(Widget *widget, HealthEventType event, void *context) {
  if (!widget_is_health(widget)) {
    return;
  }
  switch (widget->type) {
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
  widget_update_layer(widget);
}

void widget_update_weather(Widget *widget) {
  if (!widget_is_weather(widget)) {
    return;
  }
  WeatherData *w = weather_get();
  switch (widget->type) {
    case ELEMENT_WEATHER_CURRENT:
      if (w->valid) {
        snprintf(widget_temp, sizeof(widget_temp), "%d°", (int)w->temp);
      } else {
        snprintf(widget_temp, sizeof(widget_temp), "--°");
      }
      break;
    case ELEMENT_WEATHER_TODAY_MIN_MAX:
      if (w->valid) {
        snprintf(widget_temp, sizeof(widget_temp), "%d/%d°", (int)w->today_min, (int)w->today_max);
      } else {
        snprintf(widget_temp, sizeof(widget_temp), "--/--");
      }
      break;
    case ELEMENT_WEATHER_TOMORROW:
      if (w->valid) {
        snprintf(widget_temp, sizeof(widget_temp), "%d°", (int)w->tomorrow_temp);
      } else {
        snprintf(widget_temp, sizeof(widget_temp), "--°");
      }
      break;
    case ELEMENT_WEATHER_TOMORROW_MIN_MAX:
      if (w->valid) {
        snprintf(widget_temp, sizeof(widget_temp), "%d/%d°", (int)w->tomorrow_min, (int)w->tomorrow_max);
      } else {
        snprintf(widget_temp, sizeof(widget_temp), "--/--");
      }
      break;
    default:
      return;
  }
  widget_update_layer(widget);
}

void widget_update_time(Widget *widget, struct tm *t, TimeUnits units_changed) {
  if (!widget_is_time(widget)) {
    return;
  }
  switch (widget->type) {
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
  widget_update_layer(widget);
}

void widget_refresh(Widget *widget) {
  widget_update_any(widget);
  layer_mark_dirty(text_layer_get_layer(widget->layer));
}