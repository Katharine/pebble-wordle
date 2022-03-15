#ifndef DISTRIBUTION_LAYER_H
#define DISTRIBUTION_LAYER_H

#include <pebble.h>
#include "stat_tracker.h"

typedef Layer DistributionLayer;

DistributionLayer *distribution_layer_create(GRect frame, StatTracker *stat_tracker);
void distribution_layer_destroy(DistributionLayer *layer);

#endif
