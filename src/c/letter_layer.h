#ifndef LETTER_LAYER_H
#define LETTER_LAYER_H

#include <pebble.h>
#include "model.h"

#define LETTER_LAYER_SIZE 23

typedef Layer LetterLayer;

LetterLayer *letter_layer_create(GPoint where);
void letter_layer_destroy(LetterLayer *layer);
void letter_layer_set_letter(LetterLayer *layer, char letter, LetterStatus status, bool confirmed, int animated);

#endif