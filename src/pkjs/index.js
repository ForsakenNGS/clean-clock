var weather = require('./weather');
var keys = require('message_keys');

var DEFAULTS = {
  TIME_FORMAT: "24",
  TEMP_UNIT: "C",
  LANGUAGE: "",
  TIME_PRIMARY_COLOR: 0xFFFFFF,
  TIME_SECONDARY_COLOR: 0x000000,
  TIME_ACCENT_COLOR: 0xAA0000,
  TIME_FONT: "TEKO",
  WEATHER_INTERVAL: 4,
  WEATHER_LOCATION: "",
  TOP_PRIMARY_COLOR: 0xFFFFFF,
  TOP_SECONDARY_COLOR: 0x000000,
  TOP_ACCENT_COLOR: 0xAA0000,
  TOP_LEFT_ELEMENT: 2,
  TOP_CENTER_ELEMENT: 1,
  TOP_RIGHT_ELEMENT: 3,
  BOTTOM_PRIMARY_COLOR: 0xFFFFFF,
  BOTTOM_SECONDARY_COLOR: 0x000000,
  BOTTOM_ACCENT_COLOR: 0xAA0000,
  BOTTOM_LEFT_ELEMENT: 12,
  BOTTOM_CENTER_ELEMENT: 21,
  BOTTOM_RIGHT_ELEMENT: 14
};

function getSettings() {
  var s = localStorage.getItem('clay-settings');
  if (!s) return JSON.parse(JSON.stringify(DEFAULTS));
  try { return Object.assign({}, DEFAULTS, JSON.parse(s)); } catch(e) { return JSON.parse(JSON.stringify(DEFAULTS)); }
}

function toInt(v, def) {
  var n = parseInt(v, 10);
  return isNaN(n) ? def : n;
}

function sendSettings(settings) {
  var payload = {};
  payload[keys.TIME_FORMAT] = settings.TIME_FORMAT || "24";
  payload[keys.TEMP_UNIT] = settings.TEMP_UNIT || "C";
  payload[keys.LANGUAGE] = settings.LANGUAGE || "";
  payload[keys.TIME_PRIMARY_COLOR] = toInt(settings.TIME_PRIMARY_COLOR, 0xFFFFFF);
  payload[keys.TIME_SECONDARY_COLOR] = toInt(settings.TIME_SECONDARY_COLOR, 0x000000);
  payload[keys.TIME_ACCENT_COLOR] = toInt(settings.TIME_ACCENT_COLOR, 0xAA0000);
  payload[keys.TIME_FONT] = settings.TIME_FONT || "LECO";
  payload[keys.WEATHER_INTERVAL] = Math.max(1, Math.min(24, toInt(settings.WEATHER_INTERVAL, 1)));
  payload[keys.WEATHER_LOCATION] = settings.WEATHER_LOCATION || '';
  payload[keys.TOP_PRIMARY_COLOR] = toInt(settings.TOP_PRIMARY_COLOR, 0xFFFFFF);
  payload[keys.TOP_SECONDARY_COLOR] = toInt(settings.TOP_SECONDARY_COLOR, 0x000000);
  payload[keys.TOP_ACCENT_COLOR] = toInt(settings.TOP_ACCENT_COLOR, 0xAA0000);
  payload[keys.TOP_LEFT_ELEMENT] = toInt(settings.TOP_LEFT_ELEMENT, 0);
  payload[keys.TOP_CENTER_ELEMENT] = toInt(settings.TOP_CENTER_ELEMENT, 0);
  payload[keys.TOP_RIGHT_ELEMENT] = toInt(settings.TOP_RIGHT_ELEMENT, 0);
  payload[keys.BOTTOM_PRIMARY_COLOR] = toInt(settings.BOTTOM_PRIMARY_COLOR, 0xFFFFFF);
  payload[keys.BOTTOM_SECONDARY_COLOR] = toInt(settings.BOTTOM_SECONDARY_COLOR, 0x000000);
  payload[keys.BOTTOM_ACCENT_COLOR] = toInt(settings.BOTTOM_ACCENT_COLOR, 0xAA0000);
  payload[keys.BOTTOM_LEFT_ELEMENT] = toInt(settings.BOTTOM_LEFT_ELEMENT, 0);
  payload[keys.BOTTOM_CENTER_ELEMENT] = toInt(settings.BOTTOM_CENTER_ELEMENT, 0);
  payload[keys.BOTTOM_RIGHT_ELEMENT] = toInt(settings.BOTTOM_RIGHT_ELEMENT, 0);

  Pebble.sendAppMessage(payload, function() {
    console.log('Settings sent', payload);
  }, function(e) {
    console.log('Settings send failed: ' + JSON.stringify(e));
  });
}

function weatherIconFromYahoo(code) {
  code = parseInt(code, 10);
  if (code <= 4 || (code >= 37 && code <= 39) || code === 45) return 3;
  if ((code >= 5 && code <= 12) || code === 35) return 2;
  if ((code >= 13 && code <= 16) || (code >= 41 && code <= 43) || code === 46) return 4;
  if (code >= 19 && code <= 22) return 5;
  if ((code >= 26 && code <= 30) || code === 44) return 1;
  return 0;
}

Pebble.addEventListener('ready', function() {
  console.log('PKJS ready');
  const settings = getSettings();
  sendSettings(settings);
  weather.updateWeather(settings.WEATHER_INTERVAL);
});

Pebble.addEventListener('appmessage', function(e) {
  console.log('AppMessage received: ' + JSON.stringify(e.payload || {}));
  if (e.payload && e.payload[keys.READY] !== undefined) {
    const settings = getSettings();
    weather.updateWeather(settings.WEATHER_INTERVAL);
  }
});

Pebble.addEventListener('showConfiguration', function() {
  var settings = getSettings();
  var url = 'https://clean-clock.godlike.biz/?settings=' + encodeURIComponent(JSON.stringify(settings));
  Pebble.openURL(url);
});

Pebble.addEventListener('webviewclosed', function(e) {
  if (!e || !e.response) return;
  try {
    var settings = JSON.parse(decodeURIComponent(e.response));
    localStorage.setItem('clay-settings', JSON.stringify(settings));
    sendSettings(settings);
    weather.updateWeather(settings.WEATHER_INTERVAL, true);
  } catch (err) {
    console.log('Config parse error: ' + err);
  }
});
