#include <pebble.h>
#include "distribution_layer.h"
#include "stat_tracker.h"
#include "game.h"

typedef struct {
	StatTracker *stat_tracker;
} DistributionLayerData;

static void prv_update_proc(Layer *layer, GContext *ctx);

DistributionLayer *distribution_layer_create(GRect frame, StatTracker *stat_tracker) {
	Layer *layer = layer_create_with_data(frame, sizeof(DistributionLayerData));
	DistributionLayerData *data = layer_get_data(layer);
	data->stat_tracker = stat_tracker;
	layer_set_update_proc(layer, prv_update_proc);
	return layer;
}

void distribution_layer_destroy(DistributionLayer *layer) {
	layer_destroy(layer);
}

static void prv_update_proc(Layer *layer, GContext *ctx) {
	DistributionLayerData *data = layer_get_data(layer);

	graphics_context_set_text_color(ctx, GColorBlack);
	GRect bounds = layer_get_bounds(layer);
	int per_bar_height = bounds.size.h / GUESS_LIMIT;
	int left_pad = 12;
	int max_bar_width = bounds.size.w - left_pad;
	GFont label_font = fonts_get_system_font(FONT_KEY_GOTHIC_18);
	GFont value_font = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);

	int highest_bucket_value = 0;

	for (int i = 0; i < GUESS_LIMIT; ++i) {
		char digit[2] = {'0' + i+1, 0};
		graphics_draw_text(ctx, digit, label_font, GRect(-2, per_bar_height * i - 4, 10, per_bar_height), GTextOverflowModeWordWrap, GTextAlignmentRight, NULL);
		int value = stat_tracker_get_distribution_bucket(data->stat_tracker, i+1);
		if (value > highest_bucket_value) {
			highest_bucket_value = value;
		}
	}

	char value_str[5];
	int last_score = -1;
	if (game_get_status() == GameStatusWon) {
		last_score = game_get_guess_number();
	}
	for (int i = 0; i < GUESS_LIMIT; ++i) {
		int value = stat_tracker_get_distribution_bucket(data->stat_tracker, i+1);
		snprintf(value_str, sizeof(value_str), "%d", value);
		int width = (max_bar_width * value) / highest_bucket_value;
		if (width < 15) {
			width = 15;
		}
		graphics_context_set_fill_color(ctx, last_score == i ? GColorDarkGreen : GColorDarkGray);
		graphics_fill_rect(ctx, GRect(left_pad, per_bar_height * i + 1, width, per_bar_height - 1), 0, GCornerNone);
		graphics_context_set_text_color(ctx, GColorWhite);
		graphics_draw_text(ctx, value_str, value_font, GRect(left_pad, per_bar_height * i - 2, width - 3, per_bar_height), GTextOverflowModeWordWrap, GTextAlignmentRight, NULL);
	}
}