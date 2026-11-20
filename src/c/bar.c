#include "bar.h"
#include "settings.h"
#include "watchface.h"

void bar_load(BarSection *section, Layer *root_layer, const BarLocation location, const BarSettings bs) {
  GRect bounds = layer_get_bounds(root_layer);
  // Create bar layer and widgets
  section->location = location;
  if (location == BAR_LOCATION_TOP) {
    section->bounds = GRect(0, 0, bounds.size.w, watchface_bar_height());
  } else {
    section->bounds = GRect(0, bounds.size.h - watchface_bar_height(), bounds.size.w, watchface_bar_height());
  }
  //APP_LOG(APP_LOG_LEVEL_DEBUG, "Bar colors: (#%06X, #%06X, #%06X)", bs.primary_color, bs.secondary_color, bs.accent_color);
  const GColor color_primary = watchface_color(bs.primary_color);
  const GColor color_secondary = watchface_color(bs.secondary_color);
  const GColor color_accent = watchface_color(bs.accent_color);
  const WidgetConfig config_left = widget_config(location, WIDGET_LOCATION_LEFT, bs.left, color_primary, color_secondary, color_accent);
  const WidgetConfig config_center = widget_config(location, WIDGET_LOCATION_CENTER, bs.center, color_primary, color_secondary, color_accent);
  const WidgetConfig config_right = widget_config(location, WIDGET_LOCATION_RIGHT, bs.right, color_primary, color_secondary, color_accent);
  widget_load(&section->left, section->bounds, config_left, bar_get_widget_width(bs, WIDGET_LOCATION_LEFT, bounds));
  widget_load(&section->center, section->bounds, config_center, bar_get_widget_width(bs, WIDGET_LOCATION_CENTER, bounds));
  widget_load(&section->right, section->bounds, config_right, bar_get_widget_width(bs, WIDGET_LOCATION_RIGHT, bounds));
}

void bar_load_settings(BarSection *section, const BarSettings bs) {
  const GColor color_primary = watchface_color(bs.primary_color);
  const GColor color_secondary = watchface_color(bs.secondary_color);
  const GColor color_accent = watchface_color(bs.accent_color);
  const WidgetConfig config_left = widget_config(section->location, WIDGET_LOCATION_LEFT, bs.left, color_primary, color_secondary, color_accent);
  const WidgetConfig config_center = widget_config(section->location, WIDGET_LOCATION_CENTER, bs.center, color_primary, color_secondary, color_accent);
  const WidgetConfig config_right = widget_config(section->location, WIDGET_LOCATION_RIGHT, bs.right, color_primary, color_secondary, color_accent);
  widget_load_settings(&section->left, section->bounds, config_left, bar_get_widget_width(bs, WIDGET_LOCATION_LEFT, section->bounds));
  widget_load_settings(&section->center, section->bounds, config_center, bar_get_widget_width(bs, WIDGET_LOCATION_CENTER, section->bounds));
  widget_load_settings(&section->right, section->bounds, config_right, bar_get_widget_width(bs, WIDGET_LOCATION_RIGHT, section->bounds));
}

void bar_unload(BarSection *section) {
  widget_unload(&section->left);
  widget_unload(&section->center);
  widget_unload(&section->right);
}

void bar_update_layer(BarSection *section, Layer *layer, GContext *ctx) {
  widget_update_layer(&section->left, layer, ctx);
  widget_update_layer(&section->center, layer, ctx);
  widget_update_layer(&section->right, layer, ctx);
}

void bar_refresh(BarSection *section) {
  widget_refresh(&section->left);
  widget_refresh(&section->center);
  widget_refresh(&section->right);
}

int16_t bar_get_widget_width(const BarSettings bs, const WidgetLocation location, const GRect bounds) {
  switch (location) {
    case WIDGET_LOCATION_LEFT:
      if (bs.left == ELEMENT_NONE) {
        return bounds.size.w;
      } else {
        if (bs.center == ELEMENT_NONE) {
          if (bs.right == ELEMENT_NONE) {
            return bounds.size.w;
          } else {
            return bounds.size.w / 2;
          }
        } else {
          return (bounds.size.w - widget_get_center_width(bs.center)) / 2;
        }
      }
    case WIDGET_LOCATION_CENTER:
      if (bs.center == ELEMENT_NONE) {
        return bounds.size.w;
      } else {
        return widget_get_center_width(bs.center);
      }
    case WIDGET_LOCATION_RIGHT:
      if (bs.right == ELEMENT_NONE) {
        return bounds.size.w;
      } else {
        if (bs.center == ELEMENT_NONE) {
          if (bs.left == ELEMENT_NONE) {
            return bounds.size.w;
          } else {
            return bounds.size.w / 2;
          }
        } else {
          return (bounds.size.w - widget_get_center_width(bs.center)) / 2;
        }
      }
    default:
      return bounds.size.w;
  }
}