#include "settings.h"

#define SETTINGS_KEY 1
#define WEATHER_KEY  2

static ClaySettings s_settings;
static WeatherData s_weather;

static void prv_default_settings(void) {
  s_settings.time_format = TIME_FORMAT_24H;
  s_settings.temp_unit = TEMP_UNIT_C;
  strncpy(s_settings.language, "", sizeof(s_settings.language));
  s_settings.time_font = TIME_FONT_GARAMOND;
  s_settings.weather_interval_hours = 1;
  strncpy(s_settings.weather_location, "", sizeof(s_settings.weather_location));

  s_settings.top_primary_color = 0xFFFFFF;
  s_settings.top_secondary_color = 0x000000;
  s_settings.top_left = ELEMENT_WEEKDAY;
  s_settings.top_center = ELEMENT_DAY_OF_MONTH;
  s_settings.top_right = ELEMENT_MONTH;

  s_settings.bottom_primary_color = 0xFFFFFF;
  s_settings.bottom_secondary_color = 0x000000;
  s_settings.bottom_left = ELEMENT_WEATHER_CURRENT;
  s_settings.bottom_center = ELEMENT_BATTERY;
  s_settings.bottom_right = ELEMENT_WEATHER_TOMORROW;
}

void settings_init(void) {
  prv_default_settings();
  if (persist_exists(SETTINGS_KEY)) {
    persist_read_data(SETTINGS_KEY, &s_settings, sizeof(s_settings));
  }
  memset(&s_weather, 0, sizeof(s_weather));
  if (persist_exists(WEATHER_KEY)) {
    persist_read_data(WEATHER_KEY, &s_weather, sizeof(s_weather));
  }
}

void settings_deinit(void) {
  settings_save();
  persist_write_data(WEATHER_KEY, &s_weather, sizeof(s_weather));
}

ClaySettings *settings_get(void) {
  return &s_settings;
}

WeatherData *weather_get(void) {
  return &s_weather;
}

void settings_save(void) {
  persist_write_data(SETTINGS_KEY, &s_settings, sizeof(s_settings));
}

void weather_save(void) {
  persist_write_data(WEATHER_KEY, &s_weather, sizeof(s_weather));
}

void settings_request_weather(void) {
  DictionaryIterator *out;
  AppMessageResult res = app_message_outbox_begin(&out);
  if (res != APP_MSG_OK || !out) {
    return;
  }
  dict_write_uint8(out, MESSAGE_KEY_READY, 1);
  app_message_outbox_send();
}

static int32_t prv_color_from_tuple(Tuple *t, int32_t fallback) {
  if (!t) return fallback;
  return t->value->int32;
}

void settings_handle_inbox(DictionaryIterator *iter, void *context) {
  bool config_changed = false;
  bool weather_changed = false;

  // Time format
  Tuple *t = dict_find(iter, MESSAGE_KEY_TIME_FORMAT);
  if (t) {
      if (strcmp(t->value->cstring, "12") == 0) {
        s_settings.time_format = TIME_FORMAT_12H;
      } else {
        s_settings.time_format = TIME_FORMAT_24H;
      }
      config_changed = true;
  }
  // Temperature unit
  t = dict_find(iter, MESSAGE_KEY_TEMP_UNIT);
  if (t) {
    if (strcmp(t->value->cstring, "F") == 0) {
      s_settings.temp_unit = TEMP_UNIT_F;
    } else {
      s_settings.temp_unit = TEMP_UNIT_C;
    }
    config_changed = true;
  }
  // Language
  t = dict_find(iter, MESSAGE_KEY_LANGUAGE);
  if (t) {
    snprintf(s_settings.language, sizeof(s_settings.language), "%s", t->value->cstring);
    config_changed = true;
  }
  // Time font
  t = dict_find(iter, MESSAGE_KEY_TIME_FONT);
  if (t) {
    if (strcmp(t->value->cstring, "EBG") == 0) {
      s_settings.time_font = TIME_FONT_GARAMOND;
    } else {
      s_settings.time_font = TIME_FONT_LECO;
    }
    config_changed = true;
  }
  // Weather interval
  t = dict_find(iter, MESSAGE_KEY_WEATHER_INTERVAL);
  if (t) {
    s_settings.weather_interval_hours = t->value->int32;
    config_changed = true;
  }
  // Weather location
  t = dict_find(iter, MESSAGE_KEY_WEATHER_LOCATION);
  if (t) {
    snprintf(s_settings.weather_location, sizeof(s_settings.weather_location), "%s", t->value->cstring);
    config_changed = true;
  }

  // Top bar: Primary color
  t = dict_find(iter, MESSAGE_KEY_TOP_PRIMARY_COLOR);
  if (t) {
    s_settings.top_primary_color = prv_color_from_tuple(t, s_settings.top_primary_color);
    config_changed = true;
  }
  // Top bar: Secondary color
  t = dict_find(iter, MESSAGE_KEY_TOP_SECONDARY_COLOR);
  if (t) {
    s_settings.top_secondary_color = prv_color_from_tuple(t, s_settings.top_secondary_color);
    config_changed = true;
  }
  // Top bar: Left element
  t = dict_find(iter, MESSAGE_KEY_TOP_LEFT_ELEMENT);
  if (t) {
    s_settings.top_left = t->value->int32;
    config_changed = true;
  }
  // Top bar: Center element
  t = dict_find(iter, MESSAGE_KEY_TOP_CENTER_ELEMENT);
  if (t) {
    s_settings.top_center = t->value->int32;
    config_changed = true;
  }
  // Top bar: Right element
  t = dict_find(iter, MESSAGE_KEY_TOP_RIGHT_ELEMENT);
  if (t) {
    s_settings.top_right = t->value->int32;
    config_changed = true;
  }

  // Bottom bar: Primary color
  t = dict_find(iter, MESSAGE_KEY_BOTTOM_PRIMARY_COLOR);
  if (t) {
    s_settings.bottom_primary_color = prv_color_from_tuple(t, s_settings.bottom_primary_color);
    config_changed = true;
  }
  // Bottom bar: Secondary color
  t = dict_find(iter, MESSAGE_KEY_BOTTOM_SECONDARY_COLOR);
  if (t) {
    s_settings.bottom_secondary_color = prv_color_from_tuple(t, s_settings.bottom_secondary_color);
    config_changed = true;
  }
  // Bottom bar: Left element
  t = dict_find(iter, MESSAGE_KEY_BOTTOM_LEFT_ELEMENT);
  if (t) {
    s_settings.bottom_left = t->value->int32;
    config_changed = true;
  }
  // Bottom bar: Center element
  t = dict_find(iter, MESSAGE_KEY_BOTTOM_CENTER_ELEMENT);
  if (t) {
    s_settings.bottom_center = t->value->int32;
    config_changed = true;
  }
  // Bottom bar: Right element
  t = dict_find(iter, MESSAGE_KEY_BOTTOM_RIGHT_ELEMENT);
  if (t) {
    s_settings.bottom_right = t->value->int32;
    config_changed = true;
  }

  // Weather data
  t = dict_find(iter, MESSAGE_KEY_WEATHER_TEMP);
  if (t) {
    s_weather.temp = t->value->int32;
    s_weather.valid = true;
    weather_changed = true;
  }
  t = dict_find(iter, MESSAGE_KEY_WEATHER_ICON);
  if (t) {
    s_weather.icon = t->value->int32;
    weather_changed = true;
  }
  t = dict_find(iter, MESSAGE_KEY_WEATHER_TODAY_MIN);
  if (t) {
    s_weather.today_min = t->value->int32;
    weather_changed = true;
  }
  t = dict_find(iter, MESSAGE_KEY_WEATHER_TODAY_MAX);
  if (t) {
    s_weather.today_max = t->value->int32;
    weather_changed = true;
  }
  t = dict_find(iter, MESSAGE_KEY_WEATHER_TOMORROW_TEMP);
  if (t) {
    s_weather.tomorrow_temp = t->value->int32;
    weather_changed = true;
  }
  t = dict_find(iter, MESSAGE_KEY_WEATHER_TOMORROW_ICON);
  if (t) {
    s_weather.tomorrow_icon = t->value->int32;
    weather_changed = true;
  }
  t = dict_find(iter, MESSAGE_KEY_WEATHER_TOMORROW_MIN);
  if (t) {
    s_weather.tomorrow_min = t->value->int32;
    weather_changed = true;
  }
  t = dict_find(iter, MESSAGE_KEY_WEATHER_TOMORROW_MAX);
  if (t) {
    s_weather.tomorrow_max = t->value->int32;
    weather_changed = true;
  }

  if (config_changed) {
    settings_save();
  }
  if (weather_changed) {
    weather_save();
  }

  (void)context;
}
