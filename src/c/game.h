#ifndef GAME_H
#define GAME_H
#include "model.h"

#define GUESS_LIMIT 6

enum GameStatus {
	GameStatusPlaying,
	GameStatusWon,
	GameStatusLost,
};
typedef enum GameStatus GameStatus;

void show_game();
int game_get_number();
GameStatus game_get_status();
int game_get_guesses(LetterStatus guesses[GUESS_LIMIT][WORD_LENGTH]);

#endif
