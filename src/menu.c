#include <math.h>
#include "menu.h"
#include "dump3.h"
#include "gamePersistence.h"
#include "duColors.h"
#include "helpers.h"
#include "settings.h"

extern GameSettings game_settings;
extern int record_mode;

int game_mode_to_modifiers(int game_mode)
{
    if (game_mode == 0)
        return 0;
    if (game_mode == 1)
        return GAMEMODIFIER_BRUTAL;
    if (game_mode == 2)
        return GAMEMODIFIER_DOUBLED_SHOTS;
    if (game_mode == 3)
        return GAMEMODIFIERS_OVER_POWERUP;
    return GAMEMODIFIER_MULTIPLIED_GOLD;
}

void init_new_game(Enemy *autosave, int *mission, int *game_modifiers, int game_mode, int level_set)
{
    *game_modifiers = game_mode_to_modifiers(game_mode);
    
    if (level_set == 0)
    {
        *mission = 1;
    }
    else
    {
        *mission = game_settings.arena_config.arenas[level_set - 1].level_number;
        *game_modifiers |= GAMEMODIFIER_ARENA_FIGHT;
    }
    autosave->id = NO_OWNER;
}

int handle_menuchoice(int menuchoice, Enemy *autosave,
                      int *mission, int *game_modifiers, int slot, int game_mode, int level_set)
{
    if (menuchoice == MENUOPT_NEW_GAME)
    {
        init_new_game(autosave, mission, game_modifiers, game_mode, level_set);
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
            init_new_game(autosave, mission, game_modifiers, game_mode, 0);
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
    sprintf(help_path, DATADIR "%s\\help.dat", game_settings.mission_pack);
    FILE *f = fopen(help_path, "r");
    ALLEGRO_COLOR color = makecol(255, 255, 255);
    const int line_height = 16;
    int line = 0;
    int margin = 5;
    const int y_margin = 5;
    clear_to_color(BLACK);
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
                masked_blit(sprites, sx, sy, x, y, w, h);
            }
            if (!strcmp(cmd, "#rect"))
            {
                int x = 0, y = 0, w = 0, h = 0, r = 0, g = 0, b = 0;
                sscanf(s, "%*s %d %d %d %d %d %d %d", &x, &y, &w, &h, &r, &g, &b);
                rectfill(x, y, x + w, y + h, makecol(r, g, b));
            }
            if (!strcmp(cmd, "#margin"))
            {
                sscanf(s, "%*s %d", &margin);
            }
            if (!strcmp(cmd, "#to-line"))
            {
                sscanf(s, "%*s %d", &line);
            }
            if (!strcmp(cmd, "#page-end") || !strcmp(cmd, "#doc-end"))
            {
                //stretch_blit(buf, screen, 0, 0, 640, 480, 0, 0, SCREEN_W, SCREEN_H); TODO
                al_flip_display();
                chunkrest(300);
                while (!check_key(ALLEGRO_KEY_SPACE) && !check_key(ALLEGRO_KEY_ESCAPE))
                {
                    chunkrest(50);
                }
                clear_to_color(BLACK);
                line = 0;
                if (!strcmp(cmd, "#doc-end") || check_key(ALLEGRO_KEY_ESCAPE))
                    break;
            }
        }
        else
        {
            al_draw_textf(get_font(), color, margin, y_margin + line * line_height, 0, s);
            line++;
        }
    }
    fclose(f);
}

int menu(int ingame, Enemy *autosave, int *mission, int *game_modifiers)
{
    ArenaHighscore arena_highscore;
    access_arena_highscore(&arena_highscore, 1);

    // Make these static so that entering the menu mid-game will not forget previous choises
    static int game_mode = 0;
    static int level_set = 0;
    int slot = 0;
    int current_slot_has_save, current_slot_mission, current_slot_game_modifiers;
    peek_into_save_data(slot, &current_slot_has_save, &current_slot_mission, &current_slot_game_modifiers);
    BITMAP *menubg = al_load_bitmap(DATADIR "\\hell.jpg");

    BITMAP *sprites;
    if (!game_settings.custom_resources)
    {
        sprites = load_bitmap(DATADIR "sprites.png");
    }
    else
    {
        char path[256];
        sprintf(path, DATADIR "%s\\sprites.png", game_settings.mission_pack);
        sprites = load_bitmap(path);
    }
    MASKED_BITMAP(sprites);
    SAMPLE *s_c = load_sample(MENU_SAMPLE_FILENAME), *s_s = load_sample(MENU_SELECT_SAMPLE_FILENAME), *s_ex = load_sample(MENU_EXPLODE_FILENAME);

    ALLEGRO_TRANSFORM transform;
    al_identity_transform(&transform);
    al_scale_transform(&transform, 2, 2);
    al_use_transform(&transform);
    int c = 0, flicker = 0, wait = 0; //, show_help = 0;
    ALLEGRO_SAMPLE_ID id;

    if (record_mode == RECORD_MODE_PLAYBACK)
    {
        clear_to_color(BLACK);
        al_draw_textf(get_font(), BLACK, SCREEN_W / 2, SCREEN_H / 2, ALLEGRO_ALIGN_CENTRE, "Press enter to start demo playback");
        while (!check_key(ALLEGRO_KEY_ENTER)) chunkrest(10);
    }
    if (ingame)
    {
        c = MENUOPT_RESUME;
    }
    double bgtint = 0;
    while (!check_key(ALLEGRO_KEY_ENTER) && record_mode != RECORD_MODE_PLAYBACK)
    {
        //clear_to_color(BLACK);
        al_draw_tinted_scaled_bitmap(menubg, GRAY(50 + 20 * sin(bgtint)), 0, 0, 480, 360, 0, 0, 480 * 2, 360 * 2, 0);
        bgtint += 0.03;
        al_draw_textf(get_font(), DARK_RED, 20, 10, 0, "DESTINATION UNDERWORLD");
        al_draw_textf(get_font(), RED, 18, 12, 0, "DESTINATION UNDERWORLD");
        al_draw_textf(get_font(), WHITE, 40, 60, 0, "NEW GAME");
        al_draw_textf(get_font(), WHITE, 210, 40, 0, "< mode >");
        al_draw_textf(get_font(), WHITE, 360, 40, 0, "level set (use space to change)");
        al_draw_textf(get_font(), WHITE, 40, 100, 0, "LOAD GAME");
        al_draw_textf(get_font(), WHITE, 210, 80, 0, "< slot >");
        al_draw_textf(get_font(), WHITE, 40, 180, 0, "EXIT");
        if (ingame)
        {
            if (!(*game_modifiers & GAMEMODIFIER_ARENA_FIGHT))
                al_draw_textf(get_font(), WHITE, 40, 140, 0, "SAVE GAME");
            al_draw_textf(get_font(), WHITE, 40, 220, 0, "RESUME");
        }
        al_draw_textf(get_font(), BLUE, 10, 384, 0, "m: toggle music");
        al_draw_textf(get_font(), BLUE, 10, 410, 0, "n/p: next/previous track");
        al_draw_textf(get_font(), BLUE, 10, 436, 0, "f1: help");
        rectfill(200, 52, 345, 72, RED);
        if (game_mode == 0)
            al_draw_textf(get_font(), WHITE, 210, 60, 0, "normal");
        if (game_mode == 1)
            al_draw_textf(get_font(), WHITE, 210, 60, 0, "brutally hard");
        if (game_mode == 2)
            al_draw_textf(get_font(), WHITE, 210, 60, 0, "explosion madness");
        if (game_mode == 3)
            al_draw_textf(get_font(), WHITE, 210, 60, 0, "over power up");
        if (game_mode == 4)
            al_draw_textf(get_font(), WHITE, 210, 60, 0, "power up only");
        rectfill(360, 52, 600, 72, RED);
        if (level_set == 0)
            al_draw_textf(get_font(), WHITE, 370, 60, 0, "Main game");
            
        rectfill(360, 82, 600, 102, BLACK);
        if (level_set >= 1)
        {
            al_draw_textf(get_font(), WHITE, 370, 60, 0, "Arena: %s", game_settings.arena_config.arenas[level_set - 1].name);
            int kills = 0;
            for (int i = 0; i < ARENACONF_HIGHSCORE_MAP_SIZE; i++)
            {
                if (arena_highscore.mode[level_set - 1][i] == game_mode_to_modifiers(game_mode))
                {
                    kills = arena_highscore.kills[level_set - 1][i];
                    break;
                }
            }
            al_draw_textf(get_font(), WHITE, 370, 80, 0, "Highscore: %d", kills);
        }

        rectfill(200, 92, 220, 112, RED);
        al_draw_textf(get_font(), WHITE, 210, 100, 0, "%c", 'A' + slot);
        rectfill(235, 102, SCREEN_W, 122, BLACK);
        if (current_slot_has_save)
        {
            char game_mode_str[30];
            if (current_slot_game_modifiers == 0)
                strcpy(game_mode_str, "normal");
            if (current_slot_game_modifiers == GAMEMODIFIER_BRUTAL)
                strcpy(game_mode_str, "brutally hard");
            if (current_slot_game_modifiers == GAMEMODIFIER_DOUBLED_SHOTS)
                strcpy(game_mode_str, "explosion madness");
            if (current_slot_game_modifiers == (GAMEMODIFIER_OVERPOWERED_POWERUPS | GAMEMODIFIER_OVERPRICED_POWERUPS))
                strcpy(game_mode_str, "over power up");
            if (current_slot_game_modifiers == GAMEMODIFIER_MULTIPLIED_GOLD)
                strcpy(game_mode_str, "power up only");
            al_draw_textf(get_font(), WHITE, 240, 101, 0, "level: %d - mode: %s", current_slot_mission, game_mode_str);
        }
        else
        {
            al_draw_textf(get_font(), WHITE, 240, 101, 0, "(no save)");
        }
        if (ingame && !(*game_modifiers & GAMEMODIFIER_ARENA_FIGHT))
        {
            rectfill(200, 132, 220, 152, RED);
            al_draw_textf(get_font(), WHITE, 210, 140, 0, "%c", 'A' + slot);
        }

        if (++flicker == 16)
            flicker = -16;
        circlefill(30, c * 40 + 65, abs(flicker / 2), makecol(abs(flicker * 8) + 50, 0, 0));
        al_flip_display();
        //stretch_blit(buf, screen, 0, 0, 640, 480, 0, 0, SCREEN_W, SCREEN_H);
        /*if (show_help)
            blit(help_pic, screen, 0, 0, SCREEN_W - help_pic->w, SCREEN_H - help_pic->h, help_pic->w, help_pic->h);*/
        chunkrest(50);
        circlefill(30, c * 40 + 65, abs(flicker / 2), BLACK);
        if (wait == 0)
        {
            if (c == MENUOPT_NEW_GAME && check_key(ALLEGRO_KEY_LEFT) && game_mode > 0)
            {
                game_mode--;
                play_sample(s_c, 1, 0, 1, 0, &id);
                wait = 3;
            }
            if (c == MENUOPT_NEW_GAME && check_key(ALLEGRO_KEY_RIGHT) && game_mode < 4)
            {
                game_mode++;
                play_sample(s_c, 1, 0, 1, 0, &id);
                wait = 3;
            }
            if (c == MENUOPT_NEW_GAME && check_key(ALLEGRO_KEY_SPACE))
            {
                level_set++;
                if (level_set == 1 + game_settings.arena_config.number_of_arenas)
                    level_set = 0;
                play_sample(s_c, 1, 0, 1, 0, &id);
                wait = 3;
            }
            if ((c == MENUOPT_LOAD || c == MENUOPT_SAVE) && check_key(ALLEGRO_KEY_LEFT) && slot > 0)
            {
                slot--;
                peek_into_save_data(slot, &current_slot_has_save, &current_slot_mission, &current_slot_game_modifiers);
                play_sample(s_c, 1, 0, 1, 0, &id);
                wait = 3;
            }
            if ((c == MENUOPT_LOAD || c == MENUOPT_SAVE) && check_key(ALLEGRO_KEY_RIGHT) && slot < 9)
            {
                slot++;
                peek_into_save_data(slot, &current_slot_has_save, &current_slot_mission, &current_slot_game_modifiers);
                play_sample(s_c, 1, 0, 1, 0, &id);
                wait = 3;
            }
            if (check_key(ALLEGRO_KEY_UP) && c > MENUOPT_NEW_GAME)
            {
                c--;
                play_sample(s_c, 1, 0, 1, 0, &id);
                if ((!ingame || (*game_modifiers & GAMEMODIFIER_ARENA_FIGHT)) && c == MENUOPT_SAVE)
                    c = MENUOPT_LOAD;
                wait = 3;
            }
            if (check_key(ALLEGRO_KEY_DOWN) && c < (ingame ? MENUOPT_RESUME : MENUOPT_EXIT))
            {
                c++;
                play_sample(s_c, 1, 0, 1, 0, &id);
                if ((!ingame || (*game_modifiers & GAMEMODIFIER_ARENA_FIGHT)) && c == MENUOPT_SAVE)
                    c = MENUOPT_EXIT;
                chunkrest(100);
                wait = 3;
            }
            if (check_key(ALLEGRO_KEY_M))
            {
                game_settings.music_on = !game_settings.music_on;
                chunkrest(100);
                wait = 3;
            }
            if (check_key(ALLEGRO_KEY_N))
            {
                switch_track(get_current_track() + 1);
                chunkrest(100);
                wait = 3;
            }
            if (check_key(ALLEGRO_KEY_P))
            {
                switch_track(get_current_track() - 1);
                chunkrest(100);
                wait = 3;
            }
            if (check_key(ALLEGRO_KEY_F1))
            {
                show_help(sprites);
            }
        }
        wait = imax(wait - 1, 0);
    }
    if (record_mode != RECORD_MODE_PLAYBACK)
    {
        play_sample(s_s, 1, 0, 1, 0, &id);
        chunkrest(500);
    }
    al_destroy_bitmap(sprites);
    al_destroy_bitmap(menubg);
    al_destroy_sample(s_c);
    al_destroy_sample(s_s);
    al_destroy_sample(s_ex);

    return handle_menuchoice(c, autosave, mission, game_modifiers, slot, game_mode, level_set);
}
