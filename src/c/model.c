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

static void log_tm(struct tm *t) {
	APP_LOG(APP_LOG_LEVEL_INFO, "tm_sec: %d", t->tm_sec);
	APP_LOG(APP_LOG_LEVEL_INFO, "tm_min: %d", t->tm_min);
	APP_LOG(APP_LOG_LEVEL_INFO, "tm_hour: %d", t->tm_hour);
	APP_LOG(APP_LOG_LEVEL_INFO, "tm_mday: %d", t->tm_mday);
	APP_LOG(APP_LOG_LEVEL_INFO, "tm_year: %d", t->tm_year);
	APP_LOG(APP_LOG_LEVEL_INFO, "tm_wday: %d", t->tm_wday);
	APP_LOG(APP_LOG_LEVEL_INFO, "tm_yday: %d", t->tm_yday);
	APP_LOG(APP_LOG_LEVEL_INFO, "tm_isdst: %d", t->tm_isdst);
}

static int prv_compare_tm(struct tm *a, struct tm *b) {
	if (a->tm_year > b->tm_year) {
		return 1;
	} else if (a->tm_year < b->tm_year) {
		return -1;
	}
	if (a->tm_mon > b->tm_mon) {
		return 1;
	} else if(a->tm_mon < b->tm_mon) {
		return -1;
	}
	if (a->tm_mday > b->tm_mday) {
		return 1;
	} else if(a->tm_mday < b->tm_mday) {
		return -1;
	}
	if (a->tm_hour > b->tm_hour) {
		return 1;
	} else if (a->tm_hour < b->tm_hour) {
		return -1;
	}
	if (a->tm_min > b->tm_min) {
		return 1;
	} else if(a->tm_min < b->tm_min) {
		return -1;
	}
	return 0;
}

static time_t prv_tm_diff(struct tm *a, struct tm *b) {
	int compare = prv_compare_tm(a, b);
	if (compare == 0) {
		return 0;
	}
	if (compare < 0) {
		struct tm *temp = a;
		a = b;
		b = temp;
	}
	int difference = (a->tm_hour - b->tm_hour) * 3600 + (a->tm_min - b->tm_min) * 60;
	if(a->tm_mday != b->tm_mday) {
		difference += 86400;
	}
	if (compare > 0) {
		return difference;
	}
	return -difference;
}

static time_t prv_timezone_offset() {
	time_t now = time(NULL);
	struct tm local = *localtime(&now);
	struct tm utc = *gmtime(&now);
	return prv_tm_diff(&local, &utc);
}

int wordle_number() {
	// Wordle day 0 is June 19th, 2021 in the user's local time
	struct tm wordle_epoch = (struct tm) {.tm_mday = 19, .tm_mon = 5, .tm_year = 121, .tm_isdst = -1};
	time_t epoch_unixtime = mktime(&wordle_epoch);

	time_t now = time(NULL);
	APP_LOG(APP_LOG_LEVEL_INFO, "now: %d", now);
	time_t tz_offset = prv_timezone_offset();
	APP_LOG(APP_LOG_LEVEL_INFO, "tz_offset: %d", tz_offset);
	now += tz_offset;
	APP_LOG(APP_LOG_LEVEL_INFO, "local now: %d", now);
	int days = (int)(now - epoch_unixtime) / 86400;
	APP_LOG(APP_LOG_LEVEL_INFO, "wordle: %d", days);
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