#ifndef NOTIFY_LAYER_H
#define NOTIFY_LAYER_H

typedef Layer NotifyLayer;

NotifyLayer *notify_layer_create(char *message);
void notify_layer_destroy(NotifyLayer *layer);

#endif
