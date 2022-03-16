#include <pebble.h>
#include "letter_layer.h"
#include "model.h"


typedef struct {
	char letter;
	LetterStatus status;
	bool confirmed;
	Animation *sequence;
} AnimationContext;

typedef struct {
	char letter;
	LetterStatus status;
	AnimationContext animation;
	GRect large_rect;
	GRect shrunk_rect;
	bool confirmed;
} LetterLayerData;

static GColor s_layer_colors[LetterStatusCount] = {
	GColorWhite,
	PBL_IF_COLOR_ELSE(GColorLightGray, GColorDarkGray),
	PBL_IF_COLOR_ELSE(GColorYellow, GColorWhite),
	PBL_IF_COLOR_ELSE(GColorGreen, GColorBlack),
};

static GColor s_text_colors[LetterStatusCount] = {
	GColorBlack,
	GColorBlack,
	GColorBlack,
	PBL_IF_COLOR_ELSE(GColorBlack, GColorWhite),
};

static void prv_update_proc(Layer *layer, GContext *ctx);
static char prv_to_upper(char lower);
static void prv_handle_shrink_finished(Animation *animation, bool finished, void *context);
static void prv_handle_sequence_finished(Animation *animation, bool finished, void *context);
static void prv_fill_rect(Layer *layer, GContext *ctx, GColor color);

LetterLayer *letter_layer_create(GPoint where) {
	Layer *layer = layer_create_with_data(GRect(where.x, where.y, LETTER_LAYER_SIZE, LETTER_LAYER_SIZE), sizeof(LetterLayerData));
	LetterLayerData *data = layer_get_data(layer);
	data->status = LetterStatusNeutral;
	data->letter = 0;
	layer_set_update_proc(layer, prv_update_proc);
	return layer;
}

void letter_layer_set_letter(LetterLayer *layer, char letter, LetterStatus status, bool confirmed, int animated) {
	LetterLayerData *data = layer_get_data(layer);
	if (animated) {
		data->large_rect = layer_get_frame(layer);
		data->shrunk_rect = GRect(data->large_rect.origin.x, data->large_rect.origin.y + LETTER_LAYER_SIZE / 2 - 1, LETTER_LAYER_SIZE, 2);
		PropertyAnimation *shrink = property_animation_create_layer_frame(layer, &data->large_rect, &data->shrunk_rect);
		PropertyAnimation *enlarge = property_animation_create_layer_frame(layer, &data->shrunk_rect, &data->large_rect);
		animation_set_duration((Animation *)shrink, 150);
		animation_set_duration((Animation *)enlarge, 150);
		animation_set_curve((Animation *)shrink, AnimationCurveLinear);
		animation_set_curve((Animation *)enlarge, AnimationCurveLinear);
		animation_set_delay((Animation *)shrink, (animated - 1) * animation_get_duration((Animation *)shrink, false, false) * 2);
		animation_set_handlers((Animation *)shrink, (AnimationHandlers) { .stopped = prv_handle_shrink_finished }, layer);
		Animation *sequence = animation_sequence_create((Animation *)shrink, (Animation *)enlarge, NULL);
		animation_set_handlers((Animation *)sequence, (AnimationHandlers) { .stopped = prv_handle_sequence_finished }, layer);
		data->animation.letter = letter;
		data->animation.status = status;
		data->animation.confirmed = confirmed;
		data->animation.sequence = sequence;
		animation_schedule((Animation *)data->animation.sequence);
	} else {
		data->letter = letter;
		data->status = status;
		data->confirmed = confirmed;
		layer_mark_dirty(layer);
	}
}

static void prv_handle_shrink_finished(Animation *animation, bool finished, void *context) {
	LetterLayer *layer = (LetterLayer *)context;
	LetterLayerData *data = layer_get_data(layer);
	data->letter = data->animation.letter;
	data->status = data->animation.status;
	data->confirmed = data->animation.confirmed;
}

static void prv_handle_sequence_finished(Animation *animation, bool finished, void *context) {
	LetterLayer *layer = (LetterLayer *)context;
	LetterLayerData *data = layer_get_data(layer);
	animation_destroy((Animation *)data->animation.sequence);
}

void letter_layer_destroy(LetterLayer *layer) {
	layer_destroy(layer);
}

static void prv_update_proc(Layer *layer, GContext *ctx) {
	LetterLayerData *data = layer_get_data(layer);

	GRect bounds = layer_get_bounds(layer);
	graphics_context_set_stroke_color(ctx, GColorBlack);
	if (data->confirmed) {
		graphics_context_set_fill_color(ctx, s_layer_colors[data->status]);
		#ifdef PBL_COLOR
		graphics_fill_rect(ctx, GRect(1, 1, bounds.size.w - 2, bounds.size.h - 2), 0, GCornerNone);
		#else
		if (data->status == LetterStatusWrongPosition) {
			graphics_draw_rect(ctx, GRect(1, 1, bounds.size.w - 2, bounds.size.h - 2));
		} else {
			graphics_fill_rect(ctx, GRect(1, 1, bounds.size.w - 2, bounds.size.h - 2), 0, GCornerNone);
		}
		#endif
	} else {
		#ifdef PBL_COLOR
		prv_fill_rect(layer, ctx, s_layer_colors[data->status]);
		#endif
	}
	graphics_draw_rect(ctx, GRect(0, 0, bounds.size.w, bounds.size.h));

	if (data->letter != 0) {
		char letter_string[2] = {prv_to_upper(data->letter), 0};
		graphics_context_set_text_color(ctx, PBL_IF_COLOR_ELSE(true, data->confirmed) ? s_text_colors[data->status] : GColorBlack);
		graphics_draw_text(ctx, letter_string, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), GRect(0, -(LETTER_LAYER_SIZE - bounds.size.h)/2 - 1, bounds.size.w, bounds.size.h), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
	}
}

static void prv_fill_rect(Layer *layer, GContext *ctx, GColor color) {
	GBitmap *fb_bitmap = graphics_capture_frame_buffer_format(ctx, GBitmapFormat8Bit);
	uint8_t *fb = gbitmap_get_data(fb_bitmap);
	GRect bounds = layer_convert_rect_to_screen(layer, layer_get_bounds(layer));
	int screen_width = gbitmap_get_bytes_per_row(fb_bitmap);
	for (int i = bounds.origin.x; i < bounds.origin.x + bounds.size.w; ++i) {
		for (int j = bounds.origin.y; j < bounds.origin.y + bounds.size.h; ++j) {
			if ((i % 2 == 1 && j % 2 == 1) || (i % 2 == 0 && j % 2 == 0)) {
				fb[j * screen_width + i] = color.argb;
			}
		}
	}
	graphics_release_frame_buffer(ctx, fb_bitmap);
}

static char prv_to_upper(char lower) {
	return lower - ('a' - 'A');
}