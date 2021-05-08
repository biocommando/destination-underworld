#include "menu.h"
#include "dump3.h"
#include "gamePersistence.h"
#include "duColors.h"
#include "helpers.h"

extern int music_on;
extern MP3FILE *mp3;

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
        sprintf(filename, SAVE_FILENAME, slot);
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

int menu(int ingame, Enemy *autosave, int *mission, int *game_modifiers)
{
    int game_mode = 0;
    int slot = 0;
    int current_slot_has_save, current_slot_mission, current_slot_game_modifiers;
    peek_into_save_data(slot, &current_slot_has_save, &current_slot_mission, &current_slot_game_modifiers);

    BITMAP *menu_bg = create_bitmap(640, 480), *menupic = load_bitmap(MENU_BITMAP_FILENAME, default_palette), *buf = create_bitmap(640, 480);
    BITMAP *help_pic = load_bitmap(HELP_BITMAP_FILENAME, default_palette);
    SAMPLE *s_c = load_sample(MENU_SAMPLE_FILENAME), *s_s = load_sample(MENU_SELECT_SAMPLE_FILENAME), *s_ex = load_sample(MENU_EXPLODE_FILENAME);
    FONT *menufont = load_font(FONT_FILENAME, default_palette, NULL);

    int c = 0, flicker = 0, wait = 0, show_help = 0;

    // Transition animation
    clear_to_color(screen, 0);
    clear_to_color(menu_bg, 0);
    for (int i = 0; i < 10000; i++)
        putpixel(menu_bg, rand() % menu_bg->w, rand() % menu_bg->h, makecol(rand() % 20, 0, 0));
    stretch_blit(menupic, menu_bg, 0, 0, menupic->w, menupic->h, menu_bg->w - 2 * menupic->w, menu_bg->h - 2 * menupic->h, 2 * menupic->w, 2 * menupic->h);
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
    textprintf_ex(menu_bg, menufont, 10, 400, RED, -1, "m: mute music");
    textprintf_ex(menu_bg, menufont, 10, 416, RED, -1, "f1: help");
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
        if (show_help)
            blit(help_pic, screen, 0, 0, screen->w - help_pic->w, screen->h - help_pic->h, help_pic->w, help_pic->h);
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
                music_on = 1 - music_on;
                chunkrest(100);
                wait = 3;
            }
            if (key[KEY_F1])
            {
                show_help = !show_help;
                wait = 3;
            }
        }
        wait = imax(wait - 1, 0);
    }
    play_sample(s_s, 255, 127, 1000, 0);
    chunkrest(500);
    destroy_bitmap(menu_bg);
    destroy_bitmap(help_pic);
    destroy_bitmap(buf);
    destroy_sample(s_c);
    destroy_sample(s_s);
    destroy_sample(s_ex);
    destroy_font(menufont);

    return handle_menuchoice(c, autosave, mission, game_modifiers, slot, game_mode);
}
