#ifndef CURSOR_LAYER_H
#define CURSOR_LAYER_H

#include <pebble.h>

typedef Layer CursorLayer;

CursorLayer *cursor_layer_create(GPoint where);
void cursor_layer_destroy(CursorLayer *layer);

#endif
