#ifndef TITLE_LAYER_H
#define TITLE_LAYER_H

typedef Layer TitleLayer;

TitleLayer *title_layer_create(GPoint where, bool animated);
void title_layer_destroy(TitleLayer *layer);

#endif
