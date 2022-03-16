#include <pebble.h>
#include "game.h"
#include "model.h"
#include "word_layer.h"
#include "notify_layer.h"
#include "stat_tracker.h"
#include "stat_window.h"

typedef struct {
	int guess_number;
	int current_char;
	int word_number;
	GameStatus status;
	char guesses[GUESS_LIMIT][WORD_LENGTH];
	LetterStatus alphabet_status[26];
} __attribute__((packed)) GameState;

char *VICTORY_MESSAGES[GUESS_LIMIT] = {"Genius", "Magnificent", "Impressive", "Splendid", "Great", "Phew"};

static Window *s_window;
static WordLayer *s_guess_layers[GUESS_LIMIT];
static NotifyLayer *s_notify_layer;
static GameState s_game_state;
static bool s_animation_lock;
static bool s_loaded;
static char s_word[WORD_LENGTH];


static void prv_init();
static void prv_window_load(Window *window);
static void prv_window_unload(Window *window);
static void prv_click_config_provider(void *ctx);
static void prv_handle_select(ClickRecognizerRef recognizer, void *ctx);
static void prv_handle_back(ClickRecognizerRef recognizer, void *ctx);
static void prv_handle_up(ClickRecognizerRef recognizer, void *ctx);
static void prv_handle_down(ClickRecognizerRef recognizer, void *ctx);
static void prv_cycle_letter(int direction);
static void prv_update_alphabet_status(char letter, LetterStatus status);
static void prv_notify(char* message);
static void prv_animation_complete(void *context);
static void prv_save_state();
static void prv_restore_state();
static void prv_record_result();

void game_restore() {
	prv_restore_state();
}

void show_game() {
	prv_init();
}

int game_get_number() {
	return s_game_state.word_number;
}

int game_get_guess_number() {
	return s_game_state.guess_number;
}

GameStatus game_get_status() {
	return s_game_state.status;
}

int game_get_guesses(LetterStatus guesses[GUESS_LIMIT][WORD_LENGTH]) {
	GameState *s = &s_game_state;
	for (int i = 0; i <= s->guess_number; ++i) {
		score_word(s->guesses[i], s_word, guesses[i]);
	}
	return s->guess_number+1;
}

static void prv_init() {
	s_window = window_create();
	window_set_window_handlers(s_window, (WindowHandlers) {
		.load = prv_window_load,
		.unload = prv_window_unload,
	});
	window_set_click_config_provider(s_window, prv_click_config_provider);
	window_stack_push(s_window, true);
}

static void prv_window_load(Window *window) {
	for (int i = 0; i < GUESS_LIMIT; ++i) {
		s_guess_layers[i] = word_layer_create(GPoint(7, 6 + i * (WORD_LAYER_HEIGHT + 1)));
		layer_add_child(window_get_root_layer(window), s_guess_layers[i]);
	}
	prv_restore_state();
	GameState *s = &s_game_state;
	for (int i = 0; i < s->guess_number; ++i) {
		for (int j = 0; j < WORD_LENGTH; ++j) {
			word_layer_set_letter(s_guess_layers[i], j, s->guesses[i][j], LetterStatusNeutral, false);
		}
		LetterStatus statuses[WORD_LENGTH];
		score_word(s->guesses[i], s_word, statuses);
		word_layer_apply_score(s_guess_layers[i], statuses, false);
	}
	for (int j = 0; j < WORD_LENGTH; ++j) {
		if (s->guesses[s->guess_number][j] == 0) {
			break;
		}
		word_layer_set_letter(s_guess_layers[s->guess_number], j, s->guesses[s->guess_number][j], s->alphabet_status[s->guesses[s->guess_number][j] - 'a'], s->status != GameStatusPlaying);
	}
	if (s->status == GameStatusPlaying) {
		word_layer_set_cursor(s_guess_layers[s_game_state.guess_number], s_game_state.current_char);
	}
}

static void prv_window_unload(Window *window) {
	prv_save_state();
	for (int i = 0; i < GUESS_LIMIT; ++i) {
		word_layer_destroy(s_guess_layers[i]);
	}
	if (s_notify_layer != NULL) {
		notify_layer_destroy(s_notify_layer);
		s_notify_layer = NULL;
		s_animation_lock = false;
	}
}

static void prv_save_state() {
	APP_LOG(APP_LOG_LEVEL_INFO, "prv_save_state()");
	persist_write_int(1, 1);
	persist_write_data(2, &s_game_state, sizeof(GameState));
}

static void prv_restore_state() {
	APP_LOG(APP_LOG_LEVEL_INFO, "prv_restore_state()");
	if (s_loaded) {
		APP_LOG(APP_LOG_LEVEL_INFO, "already loaded state, bailing");
		return;
	}
	GameState *s = &s_game_state;
	word_of_the_day(s_word);
	int word_number = wordle_number();
	if (persist_read_int(1) == 1) {
		APP_LOG(APP_LOG_LEVEL_INFO, "usable state shuold be present, loading...");
		persist_read_data(2, s, sizeof(GameState));
		if (s->word_number == word_number) {
			APP_LOG(APP_LOG_LEVEL_INFO, "using loaded state");
			s_loaded = true;
			return;
		}
	}
	APP_LOG(APP_LOG_LEVEL_INFO, "trashing loaded state");
	memset(s, 0, sizeof(GameState));
	s->word_number = word_number;
	s_loaded = true;
}

static void prv_click_config_provider(void *ctx) {
	window_single_click_subscribe(BUTTON_ID_SELECT, prv_handle_select);
	window_single_click_subscribe(BUTTON_ID_BACK, prv_handle_back);
	window_single_repeating_click_subscribe(BUTTON_ID_UP, 100, prv_handle_up);
	window_single_repeating_click_subscribe(BUTTON_ID_DOWN, 100, prv_handle_down);
}

static void prv_handle_up(ClickRecognizerRef recognizer, void *ctx) {
	if (s_animation_lock) {
		return;
	}
	if (s_notify_layer != NULL) {
		prv_notify(NULL);
		return;
	}
	prv_cycle_letter(1);
}

static void prv_handle_down(ClickRecognizerRef recognizer, void *ctx) {
	if (s_animation_lock) {
		return;
	}
	if (s_notify_layer != NULL) {
		prv_notify(NULL);
		return;
	}
	prv_cycle_letter(-1);
}

static void prv_cycle_letter(int direction) {
	GameState *s = &s_game_state;
	if (s->status != GameStatusPlaying) {
		return;
	}
	char c = s->guesses[s->guess_number][s->current_char];
	if (c == 0) {
		c =  direction > 0 ? 'a' : 'z';
	} else {
		c += direction;
		if (c > 'z') c = 'a';
		else if (c < 'a') c = 'z';
	}
	s->guesses[s->guess_number][s->current_char] = c;
	word_layer_set_letter(s_guess_layers[s->guess_number], s->current_char, c, s->alphabet_status[c - 'a'], false);
}

static void prv_handle_select(ClickRecognizerRef recognizer, void *ctx) {
	GameState *s = &s_game_state;
	if (s_animation_lock) {
		return;
	}
	if (s->status != GameStatusPlaying) {
		stat_window_push();
		window_stack_remove(s_window, true);
		return;
	}
	if (s_notify_layer != NULL) {
		prv_notify(NULL);
		return;
	}
	if (s->guesses[s->guess_number][s->current_char] == 0) {
		if (s->guess_number > 0) {
			char c = s->guesses[s->guess_number-1][s->current_char];
			s->guesses[s->guess_number][s->current_char] = c;
			word_layer_set_letter(s_guess_layers[s->guess_number], s->current_char, c, s->alphabet_status[c - 'a'], false);
		} else {
			vibes_short_pulse();
		}
		return;
	}
	if (s->current_char < WORD_LENGTH - 1) {
		++s->current_char;
		word_layer_set_cursor(s_guess_layers[s->guess_number], s->current_char);
		return;
	}
	if (!is_valid_word(s->guesses[s->guess_number])) {
		char message[28] = "'xxxxx' is not a valid word";
		memcpy(message + 1, s->guesses[s->guess_number], 5);
		prv_notify(message);
		vibes_short_pulse();
		return;
	}
	APP_LOG(APP_LOG_LEVEL_INFO, "Scoring word...");
	LetterStatus statuses[WORD_LENGTH];
	bool correct = score_word(s->guesses[s->guess_number], s_word, statuses);
	for (int i = 0; i < WORD_LENGTH; ++i) {
		prv_update_alphabet_status(s->guesses[s->guess_number][i], statuses[i]);
	}
	APP_LOG(APP_LOG_LEVEL_INFO, "Word was correct: %d", correct);
	APP_LOG(APP_LOG_LEVEL_INFO, "Statuses: %d%d%d%d%d", statuses[0], statuses[1], statuses[2], statuses[3], statuses[4]);
	word_layer_apply_score(s_guess_layers[s->guess_number], statuses, true);
	word_layer_set_cursor(s_guess_layers[s->guess_number], -1);
	if (correct) {
		s->status = GameStatusWon;
		prv_record_result();
	} else {
		if (s->guess_number >= GUESS_LIMIT - 1) {
			s->status = GameStatusLost;
			prv_record_result();
		} else {
			++s->guess_number;
			s->current_char = 0;
		}
	}
	s_animation_lock = true;
	app_timer_register(1500, prv_animation_complete, NULL);
}

static void prv_record_result() {
	APP_LOG(APP_LOG_LEVEL_INFO, "recording result");
	StatTracker *tracker = stat_tracker_load();
	int result = 0;

	if (s_game_state.status == GameStatusWon) {
		result = s_game_state.guess_number + 1;
	}
	stat_tracker_record_result(tracker, s_game_state.word_number, result);
	stat_tracker_destroy(tracker);
}

static void prv_animation_complete(void *context) {
	GameState *s = &s_game_state;
	s_animation_lock = false;
	if (s->status == GameStatusWon) {
		prv_notify(VICTORY_MESSAGES[s->guess_number]);
	} else if (s->status == GameStatusLost) {
		char message[] = "'XXXXX'";
		memcpy(message + 1, s_word, 5);
		prv_notify(message);
	} else {
		word_layer_set_cursor(s_guess_layers[s->guess_number], 0);
	}
}

static void prv_handle_back(ClickRecognizerRef recognizer, void *ctx) {
	if (s_animation_lock) {
		return;
	}
	GameState *s = &s_game_state;
	if (s->status != GameStatusPlaying) {
		window_stack_pop(true);
		return;
	}
	if (s_notify_layer != NULL) {
		prv_notify(NULL);
		return;
	}
	if (s->current_char > 0) {
		--s->current_char;
		word_layer_set_cursor(s_guess_layers[s->guess_number], s->current_char);
	} else {
		window_stack_pop(true);
	}
}

static void prv_update_alphabet_status(char letter, LetterStatus status) {
	int index = letter - 'a';
	if (status > s_game_state.alphabet_status[index]) {
		s_game_state.alphabet_status[index] = status;
	}
}

static void prv_notify(char *message) {
	if (s_notify_layer != NULL) {
		layer_remove_from_parent(s_notify_layer);
		notify_layer_destroy(s_notify_layer);
		s_notify_layer = NULL;
	}
	if (message == NULL) {
		return;
	}
	s_notify_layer = notify_layer_create(message);
	layer_add_child(window_get_root_layer(s_window), s_notify_layer);
}
