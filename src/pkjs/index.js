var keys = require('message_keys');

var DEFAULTS = {
  TIME_FORMAT: "24",
  TEMP_UNIT: "0",
  LANGUAGE: "",
  TIME_FONT: "LECO",
  WEATHER_INTERVAL: 4,
  WEATHER_LOCATION: "",
  TOP_PRIMARY_COLOR: 0xFFFFFF,
  TOP_SECONDARY_COLOR: 0x000000,
  TOP_LEFT_ELEMENT: 2,
  TOP_CENTER_ELEMENT: 1,
  TOP_RIGHT_ELEMENT: 3,
  BOTTOM_PRIMARY_COLOR: 0xFFFFFF,
  BOTTOM_SECONDARY_COLOR: 0x000000,
  BOTTOM_LEFT_ELEMENT: 11,
  BOTTOM_CENTER_ELEMENT: 21,
  BOTTOM_RIGHT_ELEMENT: 13
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
  payload[keys.TIME_FONT] = settings.TIME_FONT || "LECO";
  payload[keys.WEATHER_INTERVAL] = Math.max(1, Math.min(24, toInt(settings.WEATHER_INTERVAL, 1)));
  payload[keys.WEATHER_LOCATION] = settings.WEATHER_LOCATION || '';
  payload[keys.TOP_PRIMARY_COLOR] = toInt(settings.TOP_PRIMARY_COLOR, 0xFFFFFF);
  payload[keys.TOP_SECONDARY_COLOR] = toInt(settings.TOP_SECONDARY_COLOR, 0x000000);
  payload[keys.TOP_LEFT_ELEMENT] = toInt(settings.TOP_LEFT_ELEMENT, 0);
  payload[keys.TOP_CENTER_ELEMENT] = toInt(settings.TOP_CENTER_ELEMENT, 0);
  payload[keys.TOP_RIGHT_ELEMENT] = toInt(settings.TOP_RIGHT_ELEMENT, 0);
  payload[keys.BOTTOM_PRIMARY_COLOR] = toInt(settings.BOTTOM_PRIMARY_COLOR, 0xFFFFFF);
  payload[keys.BOTTOM_SECONDARY_COLOR] = toInt(settings.BOTTOM_SECONDARY_COLOR, 0x000000);
  payload[keys.BOTTOM_LEFT_ELEMENT] = toInt(settings.BOTTOM_LEFT_ELEMENT, 0);
  payload[keys.BOTTOM_CENTER_ELEMENT] = toInt(settings.BOTTOM_CENTER_ELEMENT, 0);
  payload[keys.BOTTOM_RIGHT_ELEMENT] = toInt(settings.BOTTOM_RIGHT_ELEMENT, 0);

  Pebble.sendAppMessage(payload, function() {
    console.log('Settings sent');
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

function fetchWeather() {
  var settings = getSettings();
  var loc = (settings.WEATHER_LOCATION || '').trim();
  var unit = toInt(settings.TEMP_UNIT, 0) === 1 ? 'f' : 'c';
  var place = loc ? loc : 'auto:ip';
  var yql = 'select item.condition,item.forecast from weather.forecast where woeid in (select woeid from geo.places(1) where text="' + place + '") and u="' + unit + '"';
  var url = 'https://query.yahooapis.com/v1/public/yql?format=json&q=' + encodeURIComponent(yql);

  var xhr = new XMLHttpRequest();
  xhr.onload = function() {
    try {
      var json = JSON.parse(this.responseText);
      var channel = json && json.query && json.query.results && json.query.results.channel;
      if (!channel) { console.log('No weather'); return; }
      var cond = channel.item.condition || {};
      var forecast = channel.item.forecast || [];
      var today = forecast[0] || {};
      var tomorrow = forecast[1] || {};
      var payload = {};
      payload[keys.WEATHER_TEMP] = toInt(cond.temp, 0);
      payload[keys.WEATHER_ICON] = weatherIconFromYahoo(cond.code);
      payload[keys.WEATHER_TODAY_MIN] = toInt(today.low, toInt(cond.temp, 0));
      payload[keys.WEATHER_TODAY_MAX] = toInt(today.high, toInt(cond.temp, 0));
      payload[keys.WEATHER_TOMORROW_TEMP] = toInt(tomorrow.high || tomorrow.low, toInt(cond.temp, 0));
      payload[keys.WEATHER_TOMORROW_ICON] = weatherIconFromYahoo(tomorrow.code || cond.code);
      payload[keys.WEATHER_TOMORROW_MIN] = toInt(tomorrow.low, 0);
      payload[keys.WEATHER_TOMORROW_MAX] = toInt(tomorrow.high, 0);
      Pebble.sendAppMessage(payload, function(){ console.log('Weather sent'); }, function(e){ console.log('Weather fail ' + JSON.stringify(e)); });
    } catch (e) {
      console.log('Weather parse error: ' + e);
    }
  };
  xhr.onerror = function() { console.log('Weather request error'); };
  xhr.open('GET', url);
  xhr.send();
}

Pebble.addEventListener('ready', function() {
  console.log('PKJS ready');
  sendSettings(getSettings());
  fetchWeather();
});

Pebble.addEventListener('appmessage', function(e) {
  console.log('AppMessage received: ' + JSON.stringify(e.payload || {}));
  if (e.payload && e.payload[keys.READY] !== undefined) {
    fetchWeather();
  }
});

Pebble.addEventListener('showConfiguration', function() {
  var settings = getSettings();
  var url = 'https://example.com/config-page/?settings=' + encodeURIComponent(JSON.stringify(settings));
  Pebble.openURL(url);
});

Pebble.addEventListener('webviewclosed', function(e) {
  if (!e || !e.response) return;
  try {
    var settings = JSON.parse(decodeURIComponent(e.response));
    localStorage.setItem('clay-settings', JSON.stringify(settings));
    sendSettings(settings);
    fetchWeather();
  } catch (err) {
    console.log('Config parse error: ' + err);
  }
});
