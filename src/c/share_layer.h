#include <pebble.h>
#include "model.h"
#include "game.h"

typedef Layer ShareLayer;

ShareLayer *share_layer_create(GRect frame);
void share_layer_destroy(ShareLayer *layer);
void share_layer_set_game_state(ShareLayer *layer, int wordle_num, LetterStatus guesses[GUESS_LIMIT][WORD_LENGTH]);
