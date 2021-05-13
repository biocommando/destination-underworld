#include "menu.h"
#include "dump3.h"
#include "gamePersistence.h"
#include "duColors.h"
#include "helpers.h"
#include "settings.h"

extern MP3FILE *mp3;
extern GameSettings game_settings;

void init_new_game(Enemy *autosave, int *mission, int *game_modifiers, int game_mode)
{
    if (game_mode == 0) *game_modifiers = 0;
    if (game_mode == 1) *game_modifiers = GAMEMODIFIER_BRUTAL;
    if (game_mode == 2) *game_modifiers = GAMEMODIFIER_DOUBLED_SHOTS;
    if (game_mode == 3) *game_modifiers = GAMEMODIFIER_OVERPOWERED_POWERUPS | GAMEMODIFIER_OVERPRICED_POWERUPS;
    if (game_mode == 4) *game_modifiers = GAMEMODIFIER_MULTIPLIED_GOLD;
    
    *mission = 1;
    autosave->id = NO_OWNER;
}

int handle_menuchoice(int menuchoice, Enemy *autosave, 
    int *mission, int *game_modifiers, int slot, int game_mode)
{
    if (menuchoice == MENUOPT_NEW_GAME)
    {
        init_new_game(autosave, mission, game_modifiers, game_mode);
        return 1;
    }
    if (menuchoice == MENUOPT_LOAD)
    {
        char filename[50];
        sprintf(filename, SAVE_FILENAME, game_settings.mission_pack, slot);
        FILE *f = fopen(filename, "r");
        if (f)
        {
            load_game_save_data(f, autosave, mission, game_modifiers);
            autosave->id = PLAYER_ID;
            fclose(f);
        }
        else
        {
            init_new_game(autosave, mission, game_modifiers, game_mode);
        }
        if (*mission == 1)
            autosave->id = NO_OWNER;
        return 1;
    }
    if (menuchoice == MENUOPT_SAVE)
    {
        save_game(autosave, *mission, *game_modifiers, slot);
    }
    if (menuchoice == MENUOPT_EXIT)
    {
        *mission = 0;
        return 1;
    }
    return 0;
}

void show_help(BITMAP *sprites)
{
    char help_path[256];
    sprintf(help_path, ".\\dataloss\\%s\\help.txt", game_settings.mission_pack);
    FILE *f = fopen(help_path, "r");
    int color = makecol(255, 255, 255);
    const int line_height = 16;
    int line = 0;
    int margin = 5;
    const int y_margin = 5;
    BITMAP *buf = create_bitmap(640, 480);
    clear_to_color(buf, 0);
    while (!feof(f))
    {
        char s[256];
        fgets(s, 256, f);
        if (strlen(s))
            s[strlen(s) - 1] = 0;
        if (s[0] == '#') // command
        {
            char cmd[256];
            sscanf(s, "%s", cmd);
            if (!strcmp(cmd, "#color"))
            {
                int r = 255, g = 255, b = 255;
                sscanf(s, "%*s %d %d %d", &r, &g, &b);
                color = makecol(r, g, b);
            }
            if (!strcmp(cmd, "#image"))
            {
                int sx = 0, sy = 0, w = 0, h = 0, x = 0, y = 0;
                sscanf(s, "%*s %d %d %d %d %d %d", &sx, &sy, &w, &h, &x, &y);
                masked_blit(sprites, buf, sx, sy, x, y, w, h);
            }
            if (!strcmp(cmd, "#margin"))
            {
                sscanf(s, "%*s %d", &margin);
            }
            if (!strcmp(cmd, "#page-end") || !strcmp(cmd, "#doc-end"))
            {
                stretch_blit(buf, screen, 0, 0, 640, 480, 0, 0, screen->w, screen->h);
                chunkrest(500);
                while (!key[KEY_SPACE])
                {
                    chunkrest(50);
                }
                clear_to_color(buf, 0);
                line = 0;
                if (!strcmp(cmd, "#doc-end")) break;
            }
        }
        else
        {
            textprintf_ex(buf, font, margin, y_margin + line * line_height, color, -1, s);
            line++;
        }
    }
    fclose(f);
}

int menu(int ingame, Enemy *autosave, int *mission, int *game_modifiers)
{
    int game_mode = 0;
    int slot = 0;
    int current_slot_has_save, current_slot_mission, current_slot_game_modifiers;
    peek_into_save_data(slot, &current_slot_has_save, &current_slot_mission, &current_slot_game_modifiers);

    BITMAP *menu_bg = create_bitmap(640, 480), *buf = create_bitmap(640, 480);
    BITMAP *sprites;
    if (!game_settings.custom_resources)
    {
        sprites = load_bitmap(".\\dataloss\\sprites.bmp", default_palette);
    }
    else
    {
        char path[256];
        sprintf(path, ".\\dataloss\\%s\\sprites.bmp", game_settings.mission_pack);
        sprites = load_bitmap(path, default_palette);
    }
    SAMPLE *s_c = load_sample(MENU_SAMPLE_FILENAME), *s_s = load_sample(MENU_SELECT_SAMPLE_FILENAME), *s_ex = load_sample(MENU_EXPLODE_FILENAME);
    FONT *menufont = load_font(FONT_FILENAME, default_palette, NULL);

    int c = 0, flicker = 0, wait = 0; //, show_help = 0;

    // Transition animation
    clear_to_color(screen, 0);
    clear_to_color(menu_bg, 0);
    for (int i = 0; i < 10000; i++)
        putpixel(menu_bg, rand() % menu_bg->w, rand() % menu_bg->h, makecol(rand() % 20, 0, 0));
    stretch_blit(sprites, menu_bg, 100, 0, 214, 107, menu_bg->w - 2 * 214, menu_bg->h - 2 * 107, 2 * 214, 2 * 107);
    for (int i = 0; i < 24; i++)
    {
        stretch_blit(menu_bg, screen, 0, 0, 26 * (i + 1), 480, 0, 0, screen->w, screen->h);
        chunkrest(20);
    }
    clear_to_color(screen, WHITE);
    play_sample(s_ex, 255, 127, 1000, 0);
    chunkrest(500);
    // Animation ends

    textprintf_ex(menu_bg, menufont, 20, 10, DARK_RED, -1, "DESTINATION UNDERWORLD");
    textprintf_ex(menu_bg, menufont, 18, 12, RED, -1, "DESTINATION UNDERWORLD");
    textprintf_ex(menu_bg, menufont, 40, 60, WHITE, -1, "NEW GAME");
    textprintf_ex(menu_bg, menufont, 210, 40, WHITE, -1, "< mode >");
    textprintf_ex(menu_bg, menufont, 40, 100, WHITE, -1, "LOAD GAME");
    textprintf_ex(menu_bg, menufont, 210, 80, WHITE, -1, "< slot >");
    textprintf_ex(menu_bg, menufont, 40, 180, WHITE, -1, "EXIT");
    if (ingame)
    {
        textprintf_ex(menu_bg, menufont, 40, 140, WHITE, -1, "SAVE GAME");
        textprintf_ex(menu_bg, menufont, 40, 220, WHITE, -1, "RESUME");
        c = MENUOPT_RESUME;
    }
    textprintf_ex(menu_bg, menufont, 10, 384, RED, -1, "m: toggle music");
    textprintf_ex(menu_bg, menufont, 10, 410, RED, -1, "n/p: next/previous track");
    textprintf_ex(menu_bg, menufont, 10, 436, RED, -1, "f1: help");
    blit(menu_bg, buf, 0, 0, 0, 0, 640, 480);
    stretch_blit(buf, screen, 0, 0, 640, 480, 0, 0, screen->w, screen->h);

    while (!key[KEY_ENTER])
    {
        rectfill(buf, 200, 62, 340, 82, RED);
        if (game_mode == 0) textprintf_ex(buf, menufont, 210, 60, WHITE, -1, "normal");
        if (game_mode == 1) textprintf_ex(buf, menufont, 210, 60, WHITE, -1, "brutally hard");
        if (game_mode == 2) textprintf_ex(buf, menufont, 210, 60, WHITE, -1, "explosion madness");
        if (game_mode == 3) textprintf_ex(buf, menufont, 210, 60, WHITE, -1, "over power up");
        if (game_mode == 4) textprintf_ex(buf, menufont, 210, 60, WHITE, -1, "power up only");
        
        rectfill(buf, 200, 102, 230, 122, RED);
        textprintf_ex(buf, menufont, 210, 100, WHITE, -1, "%c", 'A' + slot);
        rectfill(buf, 235, 102, screen->w, 122, 0);
        if (current_slot_has_save)
        {
         char game_mode_str[30];
         if (current_slot_game_modifiers == 0) strcpy(game_mode_str, "normal");
         if (current_slot_game_modifiers == GAMEMODIFIER_BRUTAL) strcpy(game_mode_str, "brutally hard");
         if (current_slot_game_modifiers == GAMEMODIFIER_DOUBLED_SHOTS) strcpy(game_mode_str, "explosion madness");
         if (current_slot_game_modifiers == (GAMEMODIFIER_OVERPOWERED_POWERUPS | GAMEMODIFIER_OVERPRICED_POWERUPS)) strcpy(game_mode_str, "over power up");
         if (current_slot_game_modifiers == GAMEMODIFIER_MULTIPLIED_GOLD) strcpy(game_mode_str, "power up only");
         textprintf_ex(buf, menufont, 240, 101, WHITE, -1, "level: %d - mode: %s", current_slot_mission, game_mode_str);
        }
        else
        {
         textprintf_ex(buf, menufont, 240, 101, WHITE, -1, "(no save)");
        }
        if (ingame)
        {
            rectfill(buf, 200, 142, 230, 162, RED);
            textprintf_ex(buf, menufont, 210, 140, WHITE, -1, "%c", 'A' + slot);
        }
        
        if (++flicker == 16)
            flicker = -16;
        circlefill(buf, 30, c * 40 + 70, abs(flicker / 2), makecol(abs(flicker * 8) + 50, 0, 0));
        stretch_blit(buf, screen, 0, 0, 640, 480, 0, 0, screen->w, screen->h);
        /*if (show_help)
            blit(help_pic, screen, 0, 0, screen->w - help_pic->w, screen->h - help_pic->h, help_pic->w, help_pic->h);*/
        chunkrest(50);
        circlefill(buf, 30, c * 40 + 70, abs(flicker / 2), 0);
        if (wait == 0)
        {
            if (c == MENUOPT_NEW_GAME && key[KEY_LEFT] && game_mode > 0)
            {
                game_mode--;
                play_sample(s_c, 255, 127, 1000, 0);
                wait = 3;
            }
            if (c == MENUOPT_NEW_GAME && key[KEY_RIGHT] && game_mode < 4)
            {
                game_mode++;
                play_sample(s_c, 255, 127, 1000, 0);
                wait = 3;
            }
            if ((c == MENUOPT_LOAD || c == MENUOPT_SAVE) && key[KEY_LEFT] && slot > 0)
            {
                slot--;
                peek_into_save_data(slot, &current_slot_has_save, &current_slot_mission, &current_slot_game_modifiers);
                play_sample(s_c, 255, 127, 1000, 0);
                wait = 3;
            }
            if ((c == MENUOPT_LOAD || c == MENUOPT_SAVE) && key[KEY_RIGHT] && slot < 9)
            {
                slot++;
                peek_into_save_data(slot, &current_slot_has_save, &current_slot_mission, &current_slot_game_modifiers);
                play_sample(s_c, 255, 127, 1000, 0);
                wait = 3;
            }
            if (key[KEY_UP] && c > MENUOPT_NEW_GAME)
            {
                c--;
                play_sample(s_c, 255, 127, 1000, 0);
                if (!ingame && c == MENUOPT_SAVE)
                    c = MENUOPT_LOAD;
                wait = 3;
            }
            if (key[KEY_DOWN] && c < (ingame ? MENUOPT_RESUME : MENUOPT_EXIT))
            {
                c++;
                play_sample(s_c, 255, 127, 1000, 0);
                if (!ingame && c == MENUOPT_SAVE)
                    c = MENUOPT_EXIT;
                chunkrest(100);
                wait = 3;
            }
            if (key[KEY_M])
            {
                game_settings.music_on = !game_settings.music_on;
                chunkrest(100);
                wait = 3;
            }
            if (key[KEY_N])
            {
                play_track(get_current_track() + 1);
                chunkrest(100);
                wait = 3;
            }
            if (key[KEY_P])
            {
                play_track(get_current_track() - 1);
                chunkrest(100);
                wait = 3;
            }
            if (key[KEY_F1])
            {
                //show_help = !show_help;
                //wait = 3;
                show_help(sprites);
                stretch_blit(buf, screen, 0, 0, 640, 480, 0, 0, screen->w, screen->h);
            }
        }
        wait = imax(wait - 1, 0);
    }
    play_sample(s_s, 255, 127, 1000, 0);
    chunkrest(500);
    destroy_bitmap(menu_bg);
    destroy_bitmap(sprites);
    destroy_bitmap(buf);
    destroy_sample(s_c);
    destroy_sample(s_s);
    destroy_sample(s_ex);
    destroy_font(menufont);

    return handle_menuchoice(c, autosave, mission, game_modifiers, slot, game_mode);
}
