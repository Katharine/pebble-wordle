#include <pebble.h>
#include "model.h"

static short prv_words_in_file(ResHandle file);
static short prv_accepted_words();
static void prv_accepted_at_index(int index, char buffer[WORD_LENGTH]);
static int prv_compare_words(char a[WORD_LENGTH], char b[WORD_LENGTH]);
static int prv_letter_count(char word[WORD_LENGTH], int limit, char letter);
static int prv_correct_letter_count(char word[WORD_LENGTH], char expected[WORD_LENGTH], char letter);
static void prv_word_for_day(int day, char word[WORD_LENGTH]);
static void prv_word_at_index(ResHandle file, int index, char buffer[WORD_LENGTH]);

bool score_word(char word[WORD_LENGTH], char expected[WORD_LENGTH], LetterStatus result[WORD_LENGTH]) {
	bool correct = true;

	for (int i = 0; i < WORD_LENGTH; ++i) {
		if (word[i] == expected[i]) {
			result[i] = LetterStatusCorrect;
			continue;
		}
		correct = false;
		if (prv_letter_count(expected, WORD_LENGTH, word[i]) - prv_correct_letter_count(word, expected, word[i]) > prv_letter_count(word, i, word[i])) {
			result[i] = LetterStatusWrongPosition;
		} else {
			result[i] = LetterStatusNotPresent;
		}
	}

	return correct;
}

bool is_valid_word(char word[WORD_LENGTH]) {
	int total_words = prv_accepted_words();
	int pos = total_words / 2;
	int lower_bound = 0;
	int upper_bound = total_words - 1;

	while (lower_bound <= upper_bound) {
		char comp_word[WORD_LENGTH];
		prv_accepted_at_index(pos, comp_word);
		int comp = prv_compare_words(word, comp_word);
		if (comp == 0) {
			return true;
		}
		if (comp < 0) {
			upper_bound = pos - 1;
			pos = (lower_bound + upper_bound) / 2;
		} else {
			lower_bound = pos + 1;
			pos = (lower_bound + upper_bound) / 2;
		}
	}

	return false;
}

void word_of_the_day(char word[WORD_LENGTH]) {
	prv_word_for_day(wordle_number(), word);
}

int wordle_number() {
	// Wordle day 0 is June 19th, 2021 in the user's local time
	struct tm wordle_epoch = (struct tm) {.tm_mday = 19, .tm_mon = 5, .tm_year = 121, .tm_isdst = -1};
	time_t now = time(NULL) - 86400*2;
	time_t epoch_unixtime = mktime(&wordle_epoch);

	int days = (int)(now - epoch_unixtime) / 86400;
	APP_LOG(APP_LOG_LEVEL_INFO, "wordle %d", days);
	return days;
}

static void prv_word_for_day(int day, char word[WORD_LENGTH]) {
	prv_word_at_index(resource_get_handle(RESOURCE_ID_SOLUTION_WORDS), day, word);
}

static int prv_letter_count(char word[WORD_LENGTH], int limit, char letter) {
	int count = 0;
	for (int i = 0; i < limit; ++i) {
		if (word[i] == letter) {
			++count;
		}
	}
	return count;
}

static int prv_correct_letter_count(char word[WORD_LENGTH], char expected[WORD_LENGTH], char letter) {
	int count = 0;
	for (int i = 0; i < WORD_LENGTH; ++i) {
		if (expected[i] == letter && word[i] == letter) {
			++count;
		}
	}
	return count;
}

static void prv_word_at_index(ResHandle file, int index, char buffer[WORD_LENGTH]) {
	resource_load_byte_range(file, index * WORD_LENGTH, (uint8_t*)buffer, WORD_LENGTH);
}

static short prv_words_in_file(ResHandle file) {
	return resource_size(file) / WORD_LENGTH;
}

static short prv_accepted_words() {
	return prv_words_in_file(resource_get_handle(RESOURCE_ID_ACCEPTABLE_WORDS));
}

static void prv_accepted_at_index(int index, char buffer[WORD_LENGTH]) {
	prv_word_at_index(resource_get_handle(RESOURCE_ID_ACCEPTABLE_WORDS), index, buffer);
}

static int prv_compare_words(char a[WORD_LENGTH], char b[WORD_LENGTH]) {
	for (int i = 0; i < WORD_LENGTH; ++i) {
		if (a[i] < b[i]) {
			return -1;
		}
		if (a[i] > b[i]) {
			return 1;
		}
	}
	return 0;
}