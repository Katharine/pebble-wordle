#include <pebble.h>
#include "stat_tracker.h"
#include "model.h"

#define STAT_TRACKER_VERSION_KEY 3
#define STAT_TRACKER_CONTENT_KEY 4

struct StatTracker {
	short current_streak;
	short max_streak;
	short last_wordle;
	short distribution[7];
} __attribute__((packed));

StatTracker *stat_tracker_load() {
	StatTracker *tracker = malloc(sizeof(StatTracker));
	if (persist_read_int(STAT_TRACKER_VERSION_KEY) == 1) {
		persist_read_data(STAT_TRACKER_CONTENT_KEY, tracker, sizeof(StatTracker));
	} else {
		memset(tracker, 0, sizeof(StatTracker));
	}
	return tracker;
}

void stat_tracker_destroy(StatTracker *stat_tracker) {
	free(stat_tracker);
}

void stat_tracker_record_result(StatTracker *stat_tracker, int wordle_num, int result) {
	if (result > 0) {
		if (stat_tracker->last_wordle >= wordle_num - 1) {
			stat_tracker->current_streak++;
		} else {
			stat_tracker->current_streak = 1;
		}
		if (stat_tracker->current_streak > stat_tracker->max_streak) {
			stat_tracker->max_streak = stat_tracker->current_streak;
		}
		stat_tracker->last_wordle = wordle_num;
	} else {
		stat_tracker->current_streak = 0;
	}
	stat_tracker->distribution[result]++;
	persist_write_int(STAT_TRACKER_VERSION_KEY, 1);
	persist_write_data(STAT_TRACKER_CONTENT_KEY, stat_tracker, sizeof(StatTracker));
}

int stat_tracker_get_current_streak(StatTracker *tracker) {
	return tracker->last_wordle >= wordle_number() - 1 ? tracker->current_streak : 0;
}

int stat_tracker_get_max_streak(StatTracker *tracker) {
	return tracker->max_streak;
}

int stat_tracker_get_total_played(StatTracker *tracker) {
	int total = 0;
	for (int i = 0; i < 7; ++i) {
		total += tracker->distribution[i];
	}
	return total;
}

int stat_tracker_get_win_percent(StatTracker *tracker) {
	int percent = tracker->distribution[0] * 100;
	int total_played = stat_tracker_get_total_played(tracker);
	if (total_played == 0) {
		return 100;
	}
	percent /= total_played;
	percent = 100 - percent;
	return percent;
}

int stat_tracker_get_distribution_bucket(StatTracker *tracker, int bucket) {
	return tracker->distribution[bucket];
}
