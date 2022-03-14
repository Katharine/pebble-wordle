#include <pebble.h>
#include "cursor_layer.h"
#include "letter_layer.h"

static void prv_update_proc(Layer *layer, GContext *ctx);

CursorLayer *cursor_layer_create(GPoint where) {
	Layer *layer = layer_create(GRect(where.x, where.y, LETTER_LAYER_SIZE + 2, LETTER_LAYER_SIZE + 2));
	layer_set_update_proc(layer, prv_update_proc);
	return layer;
}

void cursor_layer_destroy(CursorLayer *layer) {
	layer_destroy(layer);
}

static void prv_update_proc(Layer *layer, GContext *ctx) {
	graphics_context_set_stroke_color(ctx, GColorBlack);
	graphics_draw_rect(ctx, layer_get_bounds(layer));
}
