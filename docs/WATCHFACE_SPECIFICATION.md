# Watchface descriptions

## General structure and functionality

The watchface consists on 3 parts:
- Top bar
  - Can contain up to 3 elements (left, center, right), which are configurable
  - Default height is 48px (emery) or 36px (aplite, basalt, diorite, flint)
  - Configurable via primary and secondary color (by default primary = text/icon, secondary = background)
- Time element
  - Simple digital clock in the center of the watchface
  - Default height is 132px (emery) or 96px (aplite, basalt, diorite, flint)
  - Configurable via primary and secondary color (primary = text, secondary = background)
- Bottom bar
  - Can contain up to 3 elements (left, center, right), which are configurable
  - Default height is 48px (emery) or 36px (aplite, basalt, diorite, flint)
  - Configurable via primary and secondary color (by default primary = text/icon, secondary = background)

For the top- and bottom-bar the following elements should be available:
- Time-/Date-Elements
  - Day of Month
  - Weekday
  - Month
  - Week of Year
  - Seconds
- Weather elements
  - Current weather as icon (sunny, rain, etc.) + temperature
  - Todays temperature min/max
  - Tomorrows forecast as icon (sunny, rain, etc.) + temperature
  - Tomorrows temperature min/max
- Watch information / sensors
  - Battery state in percent
  - Heart rate

### Layout for top/bottom bar

- The left element will be aligned left, taking the available space to it's right.
- The center element is always perfectly center. The width is defined by the element that is assigned.
- The right element will be aligned right, taking the available space to it's left.
- If there's no center element the side elements will split the available width 50/50
- If there's only a single element it will take the full width

## Configuration

The following configuration options should be available:
- General
  - Time format (12 or 24 hours)
  - Temperature unit (celsius or fahrenheit)
  - Language (by default as configured in the watch)
- Time element
  - Font ("EBGaramond" or "LECO 1976")
    - Emery uses `EB_GARAMOND_120` / `LECO_120`
    - Aplite, basalt, diorite, flint uses `EB_GARAMOND_80` / `LECO_80`
- Weather elements
  - Update interval in hours (1-24)
  - Location to be used (empty = as provided by pebble app)
- Top bar
  - Primary color
  - Secondary color
  - Left element (As defined above or none)
  - Center element (As defined above or none)
  - Right element (As defined above or none)
- Bottom bar
  - Primary color
  - Secondary color
  - Left element (As defined above or none)
  - Center element (As defined above or none)
  - Right element (As defined above or none)

The files for the server-side configuration page is in the projects `config-page` directory.

## Technical implementation / details

The implementation of distinct elements should be modularized.

### Display updates / rendering

- The display should update as little as possible.
- The time segment should only update once a minute when the time actually changes
- The top and bottom bars should only do partial updates depending on the configured elements
  - The update interval for weather elements should be configurable by the user as a whole number of hours
  - If a weather update is triggered all relevant elements should be updated in a single cycle
  - Watch information / sensors should update based on according events from the SDK
    - Redraws should only happen if the value actually changed (e.g. if the SDK reports the same battery value twice, this should not trigger a redraw)
  - For Time-/Date-Elements there should be different update cycles with all elements sharing the same interval being updated in a single cycle
    - All date elements are updated once every 24 hours when the day changes
    - The seconds element should update every second