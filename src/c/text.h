#pragma once
#include <pebble.h>
#include "fctx/fctx.h"

void text_render_fit(GContext *ctx, GRect bounds, char* text, FFont* font, GColor color, GTextAlignment alignment);