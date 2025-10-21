#pragma once

#include "../../allegro_management.h"

typedef struct {
    ALLEGRO_COLOR color;
    ALLEGRO_COLOR saved_colors[10];
    ALLEGRO_BITMAP *menu_sprites;
    int line;
    int margin;
} HelpState;
