#include "bar.h"
#include "settings.h"
#include "watchface.h"

void bar_load(BarSection *section, Layer *root_layer, const BarLocation location) {
  GRect bounds = layer_get_bounds(root_layer);
  ClaySettings *s = settings_get();
  // Create bar layer and widgets
  if (location == BAR_LOCATION_TOP) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Bar rect: (%d, %d, %d, %d)", 0, 0, bounds.size.w, watchface_bar_height());
    section->layer = layer_create(GRect(0, 0, bounds.size.w, watchface_bar_height()));
    const ElementType type_left = s->top_left;
    const ElementType type_center = s->top_center;
    const ElementType type_right = s->top_right;
    const GColor color_primary = watchface_color(s->top_primary_color);
    const GColor color_secondary = watchface_color(s->top_secondary_color);
    section->left.type = type_left;
    section->center.type = type_center;
    section->right.type = type_right;
    widget_load(&section->left, section->layer, WIDGET_LOCATION_LEFT, type_left, color_primary, color_secondary, bar_get_widget_width(section, WIDGET_LOCATION_LEFT, bounds));
    widget_load(&section->center, section->layer, WIDGET_LOCATION_CENTER, type_center, color_primary, color_secondary, bar_get_widget_width(section, WIDGET_LOCATION_CENTER, bounds));
    widget_load(&section->right, section->layer, WIDGET_LOCATION_RIGHT, type_right, color_primary, color_secondary, bar_get_widget_width(section, WIDGET_LOCATION_RIGHT, bounds));
  } else {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Bar rect: (%d, %d, %d, %d)", 0, bounds.size.h - watchface_bar_height(), bounds.size.w, watchface_bar_height());
    section->layer = layer_create(GRect(0, bounds.size.h - watchface_bar_height(), bounds.size.w, watchface_bar_height()));
    const ElementType type_left = s->bottom_left;
    const ElementType type_center = s->bottom_center;
    const ElementType type_right = s->bottom_right;
    const GColor color_primary = watchface_color(s->bottom_primary_color);
    const GColor color_secondary = watchface_color(s->bottom_secondary_color);
    section->left.type = type_left;
    section->center.type = type_center;
    section->right.type = type_right;
    widget_load(&section->left, section->layer, WIDGET_LOCATION_LEFT, type_left, color_primary, color_secondary, bar_get_widget_width(section, WIDGET_LOCATION_LEFT, bounds));
    widget_load(&section->center, section->layer, WIDGET_LOCATION_CENTER, type_center, color_primary, color_secondary, bar_get_widget_width(section, WIDGET_LOCATION_CENTER, bounds));
    widget_load(&section->right, section->layer, WIDGET_LOCATION_RIGHT, type_right, color_primary, color_secondary, bar_get_widget_width(section, WIDGET_LOCATION_RIGHT, bounds));
  }
  layer_add_child(root_layer, section->layer);
}

void bar_load_settings(BarSection *section, const ElementType left, const ElementType center, const ElementType right, const GColor color_primary, const GColor color_secondary) {
  widget_load_settings(&section->left, left, WIDGET_LOCATION_LEFT, color_primary, color_secondary);
  widget_load_settings(&section->center, center, WIDGET_LOCATION_CENTER, color_primary, color_secondary);
  widget_load_settings(&section->right, right, WIDGET_LOCATION_RIGHT, color_primary, color_secondary);
}

void bar_unload(BarSection *section) {
  widget_unload(&section->left);
  widget_unload(&section->center);
  widget_unload(&section->right);
  layer_destroy(section->layer);
}

void bar_refresh(BarSection *section) {
  widget_refresh(&section->left);
  widget_refresh(&section->center);
  widget_refresh(&section->right);
}

int16_t bar_get_widget_width(BarSection *section, const WidgetLocation location, const GRect bounds) {
  switch (location) {
    case WIDGET_LOCATION_LEFT:
      if (section->left.type == ELEMENT_NONE) {
        return bounds.size.w;
      } else {
        if (section->center.type == ELEMENT_NONE) {
          if (section->right.type == ELEMENT_NONE) {
            return bounds.size.w;
          } else {
            return bounds.size.w / 2;
          }
        } else {
          return (bounds.size.w - widget_get_center_width(section->center.type)) / 2;
        }
      }
    case WIDGET_LOCATION_CENTER:
      if (section->center.type == ELEMENT_NONE) {
        return bounds.size.w;
      } else {
        return widget_get_center_width(section->center.type);
      }
    case WIDGET_LOCATION_RIGHT:
      if (section->right.type == ELEMENT_NONE) {
        return bounds.size.w;
      } else {
        if (section->center.type == ELEMENT_NONE) {
          if (section->left.type == ELEMENT_NONE) {
            return bounds.size.w;
          } else {
            return bounds.size.w / 2;
          }
        } else {
          return (bounds.size.w - widget_get_center_width(section->center.type)) / 2;
        }
      }
    default:
      return bounds.size.w;
  }
}