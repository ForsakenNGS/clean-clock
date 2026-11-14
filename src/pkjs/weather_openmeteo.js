var weatherCommon = require('./weather');

module.exports.getWeatherFromCoords = getWeatherFromCoords;

function getWeatherFromCoords(pos) {
  console.log('getting that weather');
  var lat = pos.coords.latitude;
  var lon = pos.coords.longitude;

  var url = 'https://api.open-meteo.com/v1/forecast?latitude=' +
    lat + '&longitude=' + lon +
    '&current_weather=true' +
    '&daily=temperature_2m_max,temperature_2m_min,uv_index_max,weathercode' +
      ',precipitation_probability_max' +
    '&hourly=uv_index' +
    '&forecast_days=2' +
    '&timezone=auto';

  console.log(url);

  getAndSendWeather(url);
}

function getAndSendWeather(url) {
  weatherCommon.xhrRequest(url, 'GET', function(responseText) {
    var json = null;
    try {
      json = responseText && JSON.parse(responseText);
    } catch (e) {
      json = null;
    }

    if (!json || !json.current_weather || !json.daily || !json.hourly) {
      weatherCommon.retryAfterFailure('Weather request');
      return;
    }

    // Handle current weather
    var temperature = Math.round(json.current_weather.temperature);
    var conditionCode = json.current_weather.weathercode;
    var isNight = json.current_weather.is_day !== 1;

    console.log('Current temperature is ' + temperature);
    console.log('Current condition code is ' + conditionCode);

    var iconToLoad = getIconForWeatherCode(conditionCode, isNight);

    // Handle forecast
    var todaysHigh = Math.round(json.daily.temperature_2m_max[0]);
    var todaysLow = Math.round(json.daily.temperature_2m_min[0]);
    var todaysCode = json.daily.weathercode[0];
    var tomorrowsHigh = Math.round(json.daily.temperature_2m_max[1]);
    var tomorrowsLow = Math.round(json.daily.temperature_2m_min[1]);
    var tomorrowsCode = json.daily.weathercode[1];

    function getCurrentUVFromHourly(json) {
      var now = new Date();
      var times = json.hourly.time;
      var uvValues = json.hourly.uv_index;

      var closestIndex = 0;
      var smallestDiff = Infinity;

      for (var i = 0; i < times.length; i++) {
        var t = new Date(times[i]);
        var diff = Math.abs(t - now);
        if (diff < smallestDiff) {
          smallestDiff = diff;
          closestIndex = i;
        }
      }
      return Math.round(uvValues[closestIndex]);
    }

    var uvIndex = getCurrentUVFromHourly(json);

    var precipChanceToday = null;
    var precipChanceTomorrow = null;
    if (json.daily.precipitation_probability_max) {
      precipChanceToday = Math.round(json.daily.precipitation_probability_max[0]);
      precipChanceTomorrow = Math.round(json.daily.precipitation_probability_max[1]);
    }

    console.log('Forecast high/low: ' + todaysHigh + '/' + todaysLow);
    console.log('Forecast condition code: ' + todaysCode);
    console.log('UV Index: ' + uvIndex);
    console.log('Is Night ' + isNight);
    console.log('Precip chance: ' + precipChanceToday + '%');

    var todaysIcon = getIconForWeatherCode(todaysCode);
    var tomorrowsIcon = getIconForWeatherCode(tomorrowsCode);

    var dictionary = {
      'WEATHER_TEMPERATURE': temperature,
      'WEATHER_CONDITION': iconToLoad,
      'WEATHER_TODAYS_CONDITION': todaysIcon,
      'WEATHER_TODAYS_MIN_TEMP': todaysLow,
      'WEATHER_TODAYS_MAX_TEMP': todaysHigh,
      'WEATHER_TOMORROWS_CONDITION': tomorrowsIcon,
      'WEATHER_TOMORROWS_MIN_TEMP': tomorrowsLow,
      'WEATHER_TOMORROWS_MAX_TEMP': tomorrowsHigh,
      'WEATHER_UV_INDEX': uvIndex
    };

    if (precipChanceToday !== null && !isNaN(precipChanceToday)) {
      dictionary.WEATHER_TODAYS_PRECIP_CHANCE = precipChanceToday;
    }
    if (precipChanceTomorrow !== null && !isNaN(precipChanceTomorrow)) {
      dictionary.WEATHER_TOMORROWS_PRECIP_CHANCE = precipChanceTomorrow;
    }

    console.log(JSON.stringify(dictionary));

    weatherCommon.sendWeatherToPebble(dictionary);
  });
}

function getIconForWeatherCode(code, isNight) {
  switch (code) {
    case 0:
        return isNight ? weatherCommon.icons.CLEAR_NIGHT : weatherCommon.icons.CLEAR_DAY;
    case 1:
    case 2:
        return isNight ? weatherCommon.icons.PARTLY_CLOUDY_NIGHT : weatherCommon.icons.PARTLY_CLOUDY;
    case 3:
    case 45:
    case 48:
        return weatherCommon.icons.CLOUDY_DAY;
    case 51:
    case 53:
    case 55:
    case 61:
    case 80:
        return weatherCommon.icons.LIGHT_RAIN;
    case 63:
    case 65:
    case 81:
    case 82:
        return weatherCommon.icons.HEAVY_RAIN;
    case 56:
    case 57:
    case 66:
    case 67:
        return weatherCommon.icons.RAINING_AND_SNOWING;
    case 71:
    case 77:
    case 85:
        return weatherCommon.icons.LIGHT_SNOW;
    case 73:
    case 75:
    case 86:
        return weatherCommon.icons.HEAVY_SNOW;
    case 95:
    case 96:
    case 99:
        return weatherCommon.icons.THUNDERSTORM;
    default:
        return weatherCommon.icons.WEATHER_GENERIC;
  }
}
