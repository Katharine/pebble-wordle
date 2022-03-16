#include <pebble.h>
#include "stat_window.h"
#include "stat_tracker.h"
#include "share_layer.h"
#include "distribution_layer.h"
#include "game.h"

static Window *s_window;
static ScrollLayer *s_scroll_layer;
static ContentIndicator *s_content_indicator;
static Layer *s_up_arrow, *s_down_arrow;
static TextLayer *s_played_label;
static TextLayer *s_played_number;
static char s_played_text[5];
static TextLayer *s_current_streak_label;
static TextLayer *s_current_streak_number;
static char s_current_streak_text[5];
static TextLayer *s_max_streak_label;
static TextLayer *s_max_streak_number;
static char s_max_streak_text[5];
static TextLayer *s_win_percent_label;
static TextLayer *s_win_percent_number;
static char s_win_percent_text[4];
static ShareLayer *s_share_layer;
static TextLayer *s_share_label;
static DistributionLayer *s_distribution_layer;
static TextLayer *s_distribution_label;
static StatTracker *s_tracker;

static void prv_window_load(Window *window);
static void prv_window_unload(Window *window);

void stat_window_push() {
	game_restore();
	s_window = window_create();
	window_set_window_handlers(s_window, (WindowHandlers) {
		.load = prv_window_load,
		.unload = prv_window_unload,
	});
	window_stack_push(s_window, true);
}

void stat_window_pop() {
	window_stack_remove(s_window, true);
}

static TextLayer *prv_create_label(ScrollLayer *scroll_layer, GRect rect, const char *label) {
	TextLayer *layer = text_layer_create(rect);
	scroll_layer_add_child(scroll_layer, (Layer *)layer);
	text_layer_set_font(layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
	text_layer_set_text_alignment(layer, GTextAlignmentCenter);
	text_layer_set_text(layer, label);
	text_layer_set_background_color(layer, GColorClear);
	return layer;
}

static TextLayer *prv_create_value(ScrollLayer *scroll_layer, GRect rect) {
	TextLayer *layer = text_layer_create(rect);
	scroll_layer_add_child(scroll_layer, (Layer *)layer);
	text_layer_set_font(layer, fonts_get_system_font(FONT_KEY_BITHAM_34_MEDIUM_NUMBERS));
	text_layer_set_text_alignment(layer, GTextAlignmentCenter);
	text_layer_set_background_color(layer, GColorClear);
	return layer;
}

static void prv_window_load(Window *window) {
	s_scroll_layer = scroll_layer_create(layer_get_bounds(window_get_root_layer(window)));
	layer_add_child(window_get_root_layer(window), (Layer *)s_scroll_layer);
	scroll_layer_set_paging(s_scroll_layer, true);
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

	bool has_completed_game = (game_get_status() == GameStatusWon || game_get_status() == GameStatusLost);
	bool show_qr_code = PBL_PLATFORM_TYPE_CURRENT != PlatformTypeAplite && has_completed_game;

	scroll_layer_set_content_size(s_scroll_layer, GSize(144, 168 * (show_qr_code ? 3 : 2)));

	s_played_number = prv_create_value(s_scroll_layer, GRect(0, 5, 72, 44));
	s_played_label = prv_create_label(s_scroll_layer, GRect(0, 39, 72, 25), "Played");
	s_win_percent_number = prv_create_value(s_scroll_layer, GRect(72, 5, 72, 44));
	s_win_percent_label = prv_create_label(s_scroll_layer, GRect(72, 39, 72, 25), "Win %");
	s_current_streak_number = prv_create_value(s_scroll_layer, GRect(0, 69, 72, 44));
	s_current_streak_label = prv_create_label(s_scroll_layer, GRect(0, 103, 72, 55), "Current\nStreak");
	s_max_streak_number = prv_create_value(s_scroll_layer, GRect(72, 69, 72, 44));
	s_max_streak_label = prv_create_label(s_scroll_layer, GRect(72, 103, 72, 55), "Max\nStreak");

	s_tracker = stat_tracker_load();

	snprintf(s_played_text, sizeof(s_played_text), "%d", stat_tracker_get_total_played(s_tracker));
	text_layer_set_text(s_played_number, s_played_text);
	snprintf(s_win_percent_text, sizeof(s_win_percent_text), "%d", stat_tracker_get_win_percent(s_tracker));
	text_layer_set_text(s_win_percent_number, s_win_percent_text);
	snprintf(s_current_streak_text, sizeof(s_current_streak_text), "%d", stat_tracker_get_current_streak(s_tracker));
	text_layer_set_text(s_current_streak_number, s_current_streak_text);
	snprintf(s_max_streak_text, sizeof(s_max_streak_text), "%d", stat_tracker_get_max_streak(s_tracker));
	text_layer_set_text(s_max_streak_number, s_max_streak_text);

	s_distribution_layer = distribution_layer_create(GRect(5, 200, 134, 125), s_tracker);
	scroll_layer_add_child(s_scroll_layer, s_distribution_layer);
	s_distribution_label = prv_create_label(s_scroll_layer, GRect(0, 177, 144, 25), "Distribution");

	if (show_qr_code) {
		s_share_layer = share_layer_create(GRect(0, 354, 144, 125));
		s_share_label = prv_create_label(s_scroll_layer, GRect(0, 479, 144, 25), "Scan to share score");
		LetterStatus statuses[GUESS_LIMIT][WORD_LENGTH];
		memset(statuses, 0, sizeof(statuses));
		game_get_guesses(statuses);
		share_layer_set_game_state(s_share_layer, game_get_number(), statuses);
		scroll_layer_add_child(s_scroll_layer, s_share_layer);
	}
}

static void prv_window_unload(Window *window) {
	text_layer_destroy(s_played_number);
	text_layer_destroy(s_played_label);
	text_layer_destroy(s_win_percent_number);
	text_layer_destroy(s_win_percent_label);
	text_layer_destroy(s_current_streak_number);
	text_layer_destroy(s_current_streak_label);
	text_layer_destroy(s_max_streak_label);
	text_layer_destroy(s_max_streak_number);
	distribution_layer_destroy(s_distribution_layer);
	text_layer_destroy(s_distribution_label);
	if (s_share_layer != NULL) {
		share_layer_destroy(s_share_layer);
		text_layer_destroy(s_share_label);
		s_share_layer = NULL;
	}
	layer_destroy(s_up_arrow);
	layer_destroy(s_down_arrow);
	stat_tracker_destroy(s_tracker);
	scroll_layer_destroy(s_scroll_layer);
}
