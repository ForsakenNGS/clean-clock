#pragma once
#include <pebble.h>

typedef enum {
  TIME_FORMAT_24H = 0,
  TIME_FORMAT_12H = 1,
} TimeFormat;

typedef enum {
  TEMP_UNIT_C = 0,
  TEMP_UNIT_F = 1,
} TempUnit;

typedef enum {
  TIME_FONT_TEKO = 0,
  TIME_FONT_BARLOW = 1,
  TIME_FONT_BEBAS = 2,
} TimeFont;

typedef enum {
  ELEMENT_NONE = 0,
  // Time-/Date elements
  ELEMENT_DAY_OF_MONTH = 1,
  ELEMENT_WEEKDAY = 2,
  ELEMENT_MONTH = 3,
  ELEMENT_WEEK_OF_YEAR = 4,
  ELEMENT_SECONDS = 5,
  // Weather elements
  ELEMENT_WEATHER_CURRENT = 11,
  ELEMENT_WEATHER_TODAY = 12,
  ELEMENT_WEATHER_TODAY_MIN_MAX = 13,
  ELEMENT_WEATHER_TOMORROW = 14,
  ELEMENT_WEATHER_TOMORROW_MIN_MAX = 15,
  // Watch information / sensors
  ELEMENT_BATTERY = 21,
  ELEMENT_HEART_RATE = 22,
} ElementType;

typedef struct {
  int32_t primary_color;
  int32_t secondary_color;
  int32_t accent_color;
  int32_t left;
  int32_t center;
  int32_t right;
} BarSettings;

typedef struct {
  int32_t time_format;
  int32_t temp_unit;
  char language[3];
  int32_t time_primary_color;
  int32_t time_secondary_color;
  int32_t time_accent_color;
  int32_t time_font;
  int32_t weather_interval_hours;
  char weather_location[64];
  BarSettings top;
  BarSettings bottom;
} ClaySettings;

typedef struct {
  // current weather
  int currentTemp;
  int currentCondition; // WeatherCondition, or -1 for none yet
  int currentUVIndex;
  // today's forecast
  int todaysMaxTemp;
  int todaysMinTemp;
  int todaysCondition; // WeatherCondition, or -1 for none yet
  // today's precipitation chance (%)
  int todaysChance;
  // today's sunrise/sunset, minutes after local midnight
  int sunriseMinute;
  int sunsetMinute;
  // tomorrows's forecast
  int tomorrowsMaxTemp;
  int tomorrowsMinTemp;
  int tomorrowsCondition; // WeatherCondition, or -1 for none yet
  // tomorrows's precipitation chance (%)
  int tomorrowsChance;
} WeatherData;

void settings_init(void);
void settings_deinit(void);
ClaySettings *settings_get(void);
void settings_save(void);
void settings_handle_inbox(DictionaryIterator *iter, void *context);
WeatherData *weather_get(void);
void weather_save(void);
void settings_request_weather(void);
