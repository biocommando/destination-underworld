#include "loadindicator.h"

#include <stdio.h>

#include "allegro_management.h"

void progress_load_state(const char *status, int display_load_state)
{
    static int load_state = 0;
    al_clear_to_color(al_map_rgb(0, 0, 0));
    al_draw_textf(get_font(), al_map_rgb(255, 255, 255), 100, 100, 0, "Status: %s", status);
    if (display_load_state)
    {
        al_draw_filled_rectangle(100, 120, 100 + (++load_state) * 30, 160, al_map_rgb(255, 255, 255));
    }
    al_flip_display();
}