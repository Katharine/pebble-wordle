#include <pebble.h>
#include "title_layer.h"
#include "letter_layer.h"

typedef struct {
	LetterLayer *letters[6];
	LetterStatus statuses[6];
	int last_flip;
	AppTimer *timer;
} TitleLayerData;

static char *s_wordle = "wordle";

static void prv_schedule_timer(TitleLayer *layer);
static void prv_timer_callback(void *context);

TitleLayer *title_layer_create(GPoint where, bool animated) {
	Layer *layer = layer_create_with_data(GRect(where.x, where.y, 6 * (LETTER_LAYER_SIZE + 1), LETTER_LAYER_SIZE), sizeof(TitleLayerData));
	TitleLayerData *data = layer_get_data(layer);

	for (int i = 0; i < 6; ++i) {
		data->letters[i] = letter_layer_create(GPoint(i * (LETTER_LAYER_SIZE + 1), 0));
		layer_add_child(layer, data->letters[i]);
		letter_layer_set_letter(data->letters[i], 0, LetterStatusNeutral, true, false);
		letter_layer_set_letter(data->letters[i], s_wordle[i], LetterStatusNeutral, true, i+1);
	}

	if (animated) {
		data->timer = app_timer_register(2500, prv_timer_callback, layer);
	}

	return layer;
}

void title_layer_destroy(TitleLayer *layer) {
	TitleLayerData *data = layer_get_data(layer);
	app_timer_cancel(data->timer);
	for (int i = 0; i < 6; ++i) {
		letter_layer_destroy(data->letters[i]);
	}
}

static void prv_schedule_timer(TitleLayer *layer) {
	TitleLayerData *data = layer_get_data(layer);
	data->timer = app_timer_register(rand() % 2000 + 1000, prv_timer_callback, layer);
}

static void prv_timer_callback(void *context) {
	TitleLayer *layer = (TitleLayer *)context;
	TitleLayerData *data = layer_get_data(layer);
	int i;
	do {
		i = rand() % 6;
	} while (i == data->last_flip);
	data->last_flip = i;
	LetterStatus status;
	do {
		status = rand() % LetterStatusCount;
	} while (status == data->statuses[i]);
	data->statuses[i] = status;
	letter_layer_set_letter(data->letters[i], s_wordle[i], status, true, true);
	prv_schedule_timer(layer);
}
