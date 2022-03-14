#include <pebble.h>
#include "model.h"
#include "title_layer.h"
#include "game.h"
#include "stat_window.h"

#define MENU_OPTIONS 2

static Window *s_window;
static TitleLayer *s_title;
static TextLayer *s_number;
static char *s_menu_text[MENU_OPTIONS] = {"Play", "Stats"};
static TextLayer *s_menu_options[MENU_OPTIONS];
static char s_number_text[15];
static int s_selected_option = 0;

static void prv_click_config_provider(void *ctx);
static void prv_construct_menu();
static void prv_update_menu();
static void prv_handle_select(ClickRecognizerRef recognizer, void *ctx);
static void prv_handle_up(ClickRecognizerRef recognizer, void *ctx);
static void prv_handle_down(ClickRecognizerRef recognizer, void *ctx);
static void prv_handle_scroll(int direction);
static void prv_window_load(Window *window);
static void prv_window_unload(Window *window);


static void prv_window_load(Window *window) {
  s_title = title_layer_create(GPoint(1, 5), true);
  layer_add_child(window_get_root_layer(window), s_title);
  s_number = text_layer_create(GRect(0, 151, 140, 16));
  text_layer_set_text_alignment(s_number, GTextAlignmentRight);
  snprintf(s_number_text, 15, "Wordle #%d", wordle_number());
  text_layer_set_text(s_number, s_number_text);
  layer_add_child(window_get_root_layer(window), (Layer *)s_number);
  prv_construct_menu();
}

static void prv_construct_menu() {
  for (int i = 0; i < MENU_OPTIONS; ++i) {
    s_menu_options[i] = text_layer_create(GRect(0, 50 + 40 * i, 144, 35));
    text_layer_set_text_alignment(s_menu_options[i], GTextAlignmentCenter);
    text_layer_set_text(s_menu_options[i], s_menu_text[i]);
    layer_add_child(window_get_root_layer(s_window), (Layer *)s_menu_options[i]);
  }
  prv_update_menu();
}

static void prv_update_menu() {
  for (int i = 0; i < MENU_OPTIONS; ++i) {
    if (i == s_selected_option) {
      text_layer_set_font(s_menu_options[i], fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
      text_layer_set_background_color(s_menu_options[i], GColorMintGreen);
    } else {
      text_layer_set_font(s_menu_options[i], fonts_get_system_font(FONT_KEY_GOTHIC_24));
      text_layer_set_background_color(s_menu_options[i], GColorClear);
    }
  }
}

static void prv_window_unload(Window *window) {
  title_layer_destroy(s_title);
  for (int i = 0; i < MENU_OPTIONS; ++i) {
    text_layer_destroy(s_menu_options[i]);
  }
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

static void prv_click_config_provider(void *ctx) {
  window_single_click_subscribe(BUTTON_ID_SELECT, prv_handle_select);
  window_single_click_subscribe(BUTTON_ID_UP, prv_handle_up);
  window_single_click_subscribe(BUTTON_ID_DOWN, prv_handle_down);
}

static void prv_handle_select(ClickRecognizerRef recognizer, void *ctx) {
  switch (s_selected_option) {
    case 0:
      show_game();
      break;
    case 1:
      stat_window_push();
      break;
  }
}

static void prv_handle_up(ClickRecognizerRef recognizer, void *ctx) {
  prv_handle_scroll(1);
}

static void prv_handle_down(ClickRecognizerRef recognizer, void *ctx) {
  prv_handle_scroll(-1);
}

static void prv_handle_scroll(int direction) {
  s_selected_option += direction;
  if (s_selected_option < 0) {
    s_selected_option = MENU_OPTIONS - 1;
  } else if (s_selected_option >= MENU_OPTIONS) {
    s_selected_option = 0;
  }
  prv_update_menu();
}

int main(void) {
  prv_init();
  app_event_loop();
}
