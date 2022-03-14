#ifndef WORD_LAYER_H
#define WORD_LAYER_H

#include <pebble.h>
#include "model.h"
#include "letter_layer.h"

#define WORD_LAYER_HEIGHT LETTER_LAYER_SIZE + 2

typedef Layer WordLayer;

WordLayer *word_layer_create(GPoint where);
void word_layer_destroy(WordLayer *layer);
void word_layer_set_letter(WordLayer *layer, int index, char letter, LetterStatus status, bool confirmed);
void word_layer_set_cursor(WordLayer *layer, int index);
void word_layer_apply_score(WordLayer *layer, LetterStatus statuses[WORD_LENGTH], bool animated);

#endif