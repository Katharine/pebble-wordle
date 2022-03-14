#ifndef STAT_TRACKER_H
#define STAT_TRACKER_H

struct StatTracker;
typedef struct StatTracker StatTracker;

void stat_tracker_record_result(StatTracker *tracker, int wordle_num, int result);
StatTracker *stat_tracker_load();
void stat_tracker_destroy(StatTracker *tracker);
int stat_tracker_get_current_streak(StatTracker *tracker);
int stat_tracker_get_max_streak(StatTracker *tracker);
int stat_tracker_get_total_played(StatTracker *tracker);
int stat_tracker_get_win_percent(StatTracker *tracker);
int stat_tracker_get_distribution_bucket(StatTracker *tracker, int bucket);

#endif
