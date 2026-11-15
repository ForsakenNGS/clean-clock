/*
 * Free, API-key-less geocoding for the WEATHER_LOCATION setting.
 *
 * The user enters a place name on the config page (e.g. "Berlin,DE" or
 * "New York"). This module resolves it to latitude/longitude using public
 * geocoding services that don't require an API key. Results are cached per
 * query so subsequent updates within a day don't hit the network again. On
 * failure the caller should fall back to GPS.
 */

var CACHE_KEY = 'clean_clock_geocode_cache';
var CACHE_MAX_AGE_MS = 24 * 60 * 60 * 1000; // re-resolve at most once a day

function loadCache() {
  try {
    var raw = window.localStorage.getItem(CACHE_KEY);
    if (!raw) return {};
    var parsed = JSON.parse(raw);
    return parsed && typeof parsed === 'object' ? parsed : {};
  } catch (e) {
    return {};
  }
}

function saveCache(cache) {
  try {
    window.localStorage.setItem(CACHE_KEY, JSON.stringify(cache));
  } catch (e) {
    // ignore storage failures
  }
}

// Minimal XHR wrapper so this module has no dependency on weather.js (which
// would create a circular require).
function xhrRequest(url, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.status >= 200 && this.status < 300 ? this.responseText : null);
  };
  xhr.onerror = function () {
    callback(null);
  };
  xhr.open('GET', url);
  xhr.send();
}

// Photon (komoot) — free, no API key, OSM-based. Returns [lon, lat].
function queryPhoton(query, callback) {
  var url = 'https://photon.komoot.io/api/?q=' + encodeURIComponent(query) +
    '&limit=1';
  xhrRequest(url, function (res) {
    if (!res) { callback(null); return; }
    var json;
    try { json = JSON.parse(res); } catch (e) { callback(null); return; }
    var f = json && json.features && json.features[0];
    if (!f || !f.geometry || !f.geometry.coordinates ||
        f.geometry.coordinates.length < 2) {
      callback(null);
      return;
    }
    callback({
      lat: f.geometry.coordinates[1],
      lon: f.geometry.coordinates[0]
    });
  });
}

// geocode.xyz — free, no API key, JSON endpoint. Note the misspelled
// "latt"/"longt" keys; a result with both zero means "not found".
function queryGeocodeXyz(query, callback) {
  var url = 'https://geocode.xyz/' + encodeURIComponent(query) + '?json=1';
  xhrRequest(url, function (res) {
    if (!res) { callback(null); return; }
    var json;
    try { json = JSON.parse(res); } catch (e) { callback(null); return; }
    var lat = Number(json.latt);
    var lon = Number(json.longt);
    if (isNaN(lat) || isNaN(lon) || (lat === 0 && lon === 0)) {
      callback(null);
      return;
    }
    callback({ lat: lat, lon: lon });
  });
}

// Resolve a place name to { latitude, longitude } or null on failure.
function geocode(query, callback) {
  query = (query || '').trim();
  if (!query) { callback(null); return; }

  var cache = loadCache();
  var entry = cache[query.toLowerCase()];
  if (entry && entry.lat != null && entry.lon != null &&
      (Date.now() - entry.ts) < CACHE_MAX_AGE_MS) {
    console.log('Geocode cache hit for: ' + query);
    callback({ latitude: entry.lat, longitude: entry.lon });
    return;
  }

  console.log('Geocoding: ' + query);

  function storeAndCallback(result) {
    if (result && result.lat != null && result.lon != null) {
      cache[query.toLowerCase()] = {
        lat: result.lat,
        lon: result.lon,
        ts: Date.now()
      };
      saveCache(cache);
    }
    callback(result ? { latitude: result.lat, longitude: result.lon } : null);
  }

  // Try Photon first, fall back to geocode.xyz.
  queryPhoton(query, function (r1) {
    if (r1) { storeAndCallback(r1); return; }
    queryGeocodeXyz(query, storeAndCallback);
  });
}

module.exports.geocode = geocode;