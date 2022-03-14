#include <pebble.h>
#include "word_layer.h"
#include "model.h"
#include "letter_layer.h"
#include "cursor_layer.h"

#define LETTER_PADDING 3

typedef struct {
	LetterLayer *letter_layers[WORD_LENGTH];
	char word[WORD_LENGTH];
	int cursor_pos;
	CursorLayer *cursor_layer;
} WordLayerData;

WordLayer *word_layer_create(GPoint where) {
	GRect size = GRect(where.x, where.y, (LETTER_LAYER_SIZE + LETTER_PADDING) * WORD_LENGTH - LETTER_PADDING + 2, LETTER_LAYER_SIZE + 2);
	Layer *layer = layer_create_with_data(size, sizeof(WordLayerData));
	WordLayerData *data = layer_get_data(layer);
	for (int i = 0; i < WORD_LENGTH; ++i) {
		data->letter_layers[i] = letter_layer_create(GPoint(1 + i * (LETTER_LAYER_SIZE + LETTER_PADDING), 1));
		layer_add_child(layer, data->letter_layers[i]);
	}
	data->cursor_pos = -1;
	data->cursor_layer = NULL;
	return layer;
}

void word_layer_destroy(WordLayer *layer) {
	WordLayerData *data = layer_get_data(layer);
	for (int i = 0; i < WORD_LENGTH; ++i) {
		letter_layer_destroy(data->letter_layers[i]);
	}
	if (data->cursor_layer) {
		cursor_layer_destroy(data->cursor_layer);
		data->cursor_layer = NULL;
	}
	layer_destroy(layer);
}

void word_layer_set_letter(WordLayer *layer, int index, char letter, LetterStatus status, bool confirmed) {
	WordLayerData *data = layer_get_data(layer);
	data->word[index] = letter;
	letter_layer_set_letter(data->letter_layers[index], letter, status, confirmed, false);
}

void word_layer_set_cursor(WordLayer *layer, int index) {
	WordLayerData *data = layer_get_data(layer);
	data->cursor_pos = index;
	if (data->cursor_layer) {
		layer_remove_from_parent(data->cursor_layer);
		cursor_layer_destroy(data->cursor_layer);
		data->cursor_layer = NULL;
	}
	if (index >= 0) {
		data->cursor_layer = cursor_layer_create(GPoint(index * (LETTER_LAYER_SIZE + LETTER_PADDING), 0));
		layer_add_child(layer, data->cursor_layer);
	} 
}

void word_layer_apply_score(WordLayer *layer, LetterStatus statuses[WORD_LENGTH], bool animated) {
	WordLayerData *data = layer_get_data(layer);
	for (int i = 0; i < WORD_LENGTH; ++i) {
		letter_layer_set_letter(data->letter_layers[i], data->word[i], statuses[i], true, animated ? i + 1 : 0);
	}
}
