#include <pebble.h>
#include "notify_layer.h"


typedef struct {
	char *message;
} NotifyLayerData;

static GFont s_font;
static void prv_update_proc(Layer *layer, GContext *ctx);

NotifyLayer *notify_layer_create(char *message) {
	if (s_font == NULL) {
		s_font = fonts_get_system_font(FONT_KEY_GOTHIC_24);
	}
	GSize size = graphics_text_layout_get_content_size(message, s_font, GRect(0, 0, 104, 148), GTextOverflowModeWordWrap, GTextAlignmentCenter);
	size.w += 10;
	size.h += 10;
	Layer *layer = layer_create_with_data(GRect((144 - size.w) / 2, (168 - size.h) / 2, size.w, size.h), sizeof(NotifyLayerData));
	layer_set_update_proc(layer, prv_update_proc);
	NotifyLayerData *data = layer_get_data(layer);
	data->message = malloc(strlen(message) + 1);
	strcpy(data->message, message);
	return layer;
}

void notify_layer_destroy(NotifyLayer *layer) {
	NotifyLayerData *data = layer_get_data(layer);
	free(data->message);
	layer_destroy(layer);
}

static void prv_update_proc(Layer *layer, GContext *ctx) {
	NotifyLayerData *data = layer_get_data(layer);
	GRect bounds = layer_get_bounds(layer);
	graphics_context_set_fill_color(ctx, GColorBlack);
	graphics_fill_rect(ctx, bounds, 5, GCornersAll);
	graphics_context_set_fill_color(ctx, GColorWhite);
	graphics_fill_rect(ctx, GRect(bounds.origin.x + 2, bounds.origin.y + 2, bounds.size.w - 4, bounds.size.h - 4), 5, GCornersAll);
	graphics_context_set_text_color(ctx, GColorBlack);
	graphics_draw_text(ctx, data->message, s_font, GRect(bounds.origin.x + 5, bounds.origin.y, bounds.size.w - 10, bounds.size.h - 10), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
}
