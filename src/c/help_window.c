#include <pebble.h>
#include "help_window.h"
#include "word_layer.h"

static Window *s_window;
static ScrollLayer *s_scroll_layer;
static ContentIndicator *s_content_indicator;
static Layer *s_down_arrow;
static Layer *s_up_arrow;
static TextLayer *s_explanation_layer;
static WordLayer *s_example1_word;
static TextLayer *s_example1_label;
static WordLayer *s_example2_word;
static TextLayer *s_example2_label;
static WordLayer *s_example3_word;
static TextLayer *s_example3_label;

static void prv_window_load(Window *window);
static void prv_window_unload(Window *window);

void help_window_push() {
	s_window = window_create();
	window_set_window_handlers(s_window, (WindowHandlers) {
		.load = prv_window_load,
		.unload = prv_window_unload,
	});
	window_stack_push(s_window, true);
}

static void prv_window_load(Window *window) {
	s_scroll_layer = scroll_layer_create(layer_get_bounds(window_get_root_layer(window)));
	layer_add_child(window_get_root_layer(window), (Layer *)s_scroll_layer);
	scroll_layer_set_shadow_hidden(s_scroll_layer, true);
	scroll_layer_set_click_config_onto_window(s_scroll_layer, window);

	s_content_indicator = scroll_layer_get_content_indicator(s_scroll_layer);
	s_up_arrow = layer_create(GRect(0, 0, 144, 15));
	s_down_arrow = layer_create(GRect(0, 153, 144, 15));
	layer_add_child(window_get_root_layer(window), s_up_arrow);
	layer_add_child(window_get_root_layer(window), s_down_arrow); 

	const ContentIndicatorConfig up_config = (ContentIndicatorConfig) {
		.layer = s_up_arrow,
		.times_out = false,
		.alignment = GAlignCenter,
		.colors = {
			.foreground = GColorBlack,
			.background = GColorWhite
		}
	};
	content_indicator_configure_direction(s_content_indicator, ContentIndicatorDirectionUp, &up_config);

	const ContentIndicatorConfig down_config = (ContentIndicatorConfig) {
		.layer = s_down_arrow,
		.times_out = false,
		.alignment = GAlignCenter,
		.colors = {
			.foreground = GColorBlack,
			.background = GColorWhite
		}
	};
	content_indicator_configure_direction(s_content_indicator, ContentIndicatorDirectionDown, &down_config);

	scroll_layer_set_content_size(s_scroll_layer, GSize(144, 1120));

	GFont font = fonts_get_system_font(FONT_KEY_GOTHIC_24);

	s_explanation_layer = text_layer_create(GRect(5, 0, 134, 685));
	text_layer_set_font(s_explanation_layer, font);
	text_layer_set_text(s_explanation_layer, "Guess the WORDLE in six tries.\n\n"
		"Each guess must be a valid five-letter word. Cycle through the letters using the up and down buttons, and hit select to confirm. "
		"You can also press select on an empty space to fill in the letter immediately above. Press back to go back a letter. "
		"Press select at the end of the word to submit.\n\n"
		"After each guess, the color of the tiles will change to show how close your guess was to the word.\n\n"
		"Examples:");
	scroll_layer_add_child(s_scroll_layer, (Layer *)s_explanation_layer);

	s_example1_word = word_layer_create(GPoint(7, 685));
	word_layer_set_letter(s_example1_word, 0, 'w', LetterStatusCorrect, true);
	word_layer_set_letter(s_example1_word, 1, 'e', LetterStatusNeutral, true);
	word_layer_set_letter(s_example1_word, 2, 'a', LetterStatusNeutral, true);
	word_layer_set_letter(s_example1_word, 3, 'r', LetterStatusNeutral, true);
	word_layer_set_letter(s_example1_word, 4, 'y', LetterStatusNeutral, true);
	scroll_layer_add_child(s_scroll_layer, (Layer *)s_example1_word);

	s_example1_label = text_layer_create(GRect(5, 710, 134, 80));
	text_layer_set_font(s_example1_label, font);
	text_layer_set_text(s_example1_label, "The letter W is in the word and in the correct spot.");
	scroll_layer_add_child(s_scroll_layer, (Layer *)s_example1_label);

	s_example2_word = word_layer_create(GPoint(7, 800));
	word_layer_set_letter(s_example2_word, 0, 'p', LetterStatusNeutral, true);
	word_layer_set_letter(s_example2_word, 1, 'i', LetterStatusWrongPosition, true);
	word_layer_set_letter(s_example2_word, 2, 'l', LetterStatusNeutral, true);
	word_layer_set_letter(s_example2_word, 3, 'l', LetterStatusNeutral, true);
	word_layer_set_letter(s_example2_word, 4, 's', LetterStatusNeutral, true);
	scroll_layer_add_child(s_scroll_layer, (Layer *)s_example2_word);

	s_example2_label = text_layer_create(GRect(5, 825, 134, 80));
	text_layer_set_font(s_example2_label, font);
	text_layer_set_text(s_example2_label, "The letter W is in the word but in the wrong spot.");
	scroll_layer_add_child(s_scroll_layer, (Layer *)s_example2_label);

	s_example3_word = word_layer_create(GPoint(7, 915));
	word_layer_set_letter(s_example3_word, 0, 'v', LetterStatusNeutral, true);
	word_layer_set_letter(s_example3_word, 1, 'a', LetterStatusNeutral, true);
	word_layer_set_letter(s_example3_word, 2, 'g', LetterStatusNeutral, true);
	word_layer_set_letter(s_example3_word, 3, 'u', LetterStatusNeutral, true);
	word_layer_set_letter(s_example3_word, 4, 'e', LetterStatusNotPresent, true);
	scroll_layer_add_child(s_scroll_layer, (Layer *)s_example3_word);

	s_example3_label = text_layer_create(GRect(5, 940, 134, 180));
	text_layer_set_font(s_example3_label, font);
	text_layer_set_text(s_example3_label, "The letter U is not in the word in any spot.\n\nA new WORDLE will be available each day!");
	scroll_layer_add_child(s_scroll_layer, (Layer *)s_example3_label);
}

static void prv_window_unload(Window *window) {
	layer_destroy(s_up_arrow);
	layer_destroy(s_down_arrow);
	word_layer_destroy(s_example1_word);
	text_layer_destroy(s_example1_label);
	word_layer_destroy(s_example2_word);
	text_layer_destroy(s_example2_label);
	word_layer_destroy(s_example3_word);
	text_layer_destroy(s_example3_label);
	text_layer_destroy(s_explanation_layer);
	scroll_layer_destroy(s_scroll_layer);
}

