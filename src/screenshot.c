
#include "screenshot.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "allegro_management.h"

inline void screenshot(int action)
{
#define BUF_LEN 16
    static ALLEGRO_BITMAP *buf[BUF_LEN];
    static int idx = 0;
    static int file_idx = 0;

    if (action == SCREENSHOT_ACT_INIT)
    {
        memset(buf, 0, sizeof(buf));
        idx = 0;
    }
    else if (action == SCREENSHOT_ACT_CAPTURE)
    {
        idx = (idx + 1) % BUF_LEN;
        if (buf[idx])
            al_destroy_bitmap(buf[idx]);
        buf[idx] = get_screen();
    }
    else if (action == SCREENSHOT_ACT_DUMP_TO_DISK)
    {
        file_idx++;
        for (int i = 0; i < BUF_LEN; i++)
        {
            int bi = (idx + i + 1) % BUF_LEN;
            if (!buf[bi])
                continue;
            char fname[30];
            sprintf(fname, "screenshot%d_%c.png", file_idx, 'A' + i);
            al_save_bitmap(fname, buf[bi]);
        }
    }
    else
    {
        for (int i = 0; i < BUF_LEN; i++)
        {
            if (buf[i])
                al_destroy_bitmap(buf[i]);
        }
    }
}