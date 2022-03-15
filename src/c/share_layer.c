#include <pebble.h>
#include "share_layer.h"
#include "game.h"
#include "model.h"
#include "vendor/qrcodegen.h"

#define URL_FORMAT "HTTPS://WORD.KTBY.IO/"

typedef struct {
	bool has_qr_code;
	uint8_t qrcode[qrcodegen_BUFFER_LEN_FOR_VERSION(2)];
} ShareLayerData;

static void prv_silly_number_encoding(int number, char* buffer);
static void prv_silly_short_encoding(uint16_t word, char* buffer);
static void prv_update_proc(Layer *layer, GContext *ctx);
static bool prv_generate_alphanumeric_qr(char *text, uint8_t *qrcode);

ShareLayer *share_layer_create(GRect frame) {
	Layer *layer = layer_create_with_data(frame, sizeof(ShareLayerData));
	layer_set_update_proc(layer, prv_update_proc);
	return layer;
}

void share_layer_set_game_state(ShareLayer *layer, int wordle_num, LetterStatus guesses[GUESS_LIMIT][WORD_LENGTH]) {
	ShareLayerData *data = layer_get_data(layer);
	char path[14];
	memset(path, 0, sizeof(path));
	prv_silly_number_encoding(wordle_num, path);
		APP_LOG(APP_LOG_LEVEL_DEBUG, "encoding: %c%c%c", path[0], path[1], path[2]);
	for (int i = 0; i < GUESS_LIMIT; ++i) {
		if (guesses[i][0] == LetterStatusNeutral) {
			break;
		}
		uint16_t guess = 0;
		for (int j = 0; j < WORD_LENGTH; ++j) {
			guess |= ((uint8_t)guesses[i][j] << (j * 2));
		}
		prv_silly_short_encoding(guess, path + 3 + i*2);
		APP_LOG(APP_LOG_LEVEL_DEBUG, "encoding: %c%c", path[3+i*2], path[4+i*2]);
	}
	size_t url_length = sizeof(URL_FORMAT) + strlen(path);
	char* url = malloc(url_length);
	snprintf(url, url_length, URL_FORMAT "%s", path);
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Encoded URL: %s", url);


	data->has_qr_code = prv_generate_alphanumeric_qr(url, data->qrcode);

	APP_LOG(APP_LOG_LEVEL_DEBUG, "Generated QR code");
	if (!data->has_qr_code) {
		goto end;
	}
end:
	free(url);
}

void share_layer_destroy(ShareLayer *layer) {
	layer_destroy(layer);
}

static bool prv_generate_alphanumeric_qr(char *text, uint8_t *qrcode) {
	uint8_t *temp_buffer = malloc(qrcodegen_BUFFER_LEN_FOR_VERSION(2));
	struct qrcodegen_Segment seg;
	seg = qrcodegen_makeAlphanumeric(text, temp_buffer);
	bool result = qrcodegen_encodeSegmentsAdvanced(&seg, 1, qrcodegen_Ecc_MEDIUM, 2, 2, qrcodegen_Mask_AUTO, true, temp_buffer, qrcode);
	free(temp_buffer);
	return result;
}

static void prv_silly_number_encoding(int number, char* buffer) {
	const char* alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
	buffer[0] = alphabet[number / (36 * 36)];
	buffer[1] = alphabet[(number % (36 * 36)) / 36];
	buffer[2] = alphabet[number % 36];
}

static void prv_silly_short_encoding(uint16_t word, char* buffer) {
	const char* alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
	if (word > 36 * 36) {
		APP_LOG(APP_LOG_LEVEL_ERROR, "%d is larger than 1296, the largest number we can encode.", word);
		buffer[0] = '9';
		buffer[1] = '9';
		return;
	}
	buffer[0] = alphabet[word / 36];
	buffer[1] = alphabet[word % 36];
}

static void prv_update_proc(Layer *layer, GContext *ctx) {
	ShareLayerData *data = layer_get_data(layer);
	if (!data->has_qr_code) {
		graphics_context_set_fill_color(ctx, GColorRed);
		graphics_fill_rect(ctx, layer_get_bounds(layer), 0, GCornerNone);
		return;
	}

	int modules_per_side = qrcodegen_getSize(data->qrcode);
	GRect bounds = layer_get_bounds(layer);
	int ppm = (bounds.size.w < bounds.size.h ? bounds.size.w : bounds.size.h) / modules_per_side;
	int offset_x = (bounds.size.w - ppm * modules_per_side) / 2;
	int offset_y = (bounds.size.h - ppm * modules_per_side) / 2;
	for (int i = 0; i < modules_per_side; ++i) {
		for (int j = 0; j < modules_per_side; ++j) {
			GColor color = qrcodegen_getModule(data->qrcode, i, j) ? GColorBlack : GColorWhite;
			graphics_context_set_fill_color(ctx, color);
			graphics_fill_rect(ctx, GRect(offset_x + i * ppm, offset_y + j * ppm, ppm, ppm), 0, GCornerNone);
		}
	}
}
