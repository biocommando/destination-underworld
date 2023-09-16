#include <math.h>
#include <stdarg.h>
#include "menu.h"
#include "dump3.h"
#include "gamePersistence.h"
#include "duColors.h"
#include "helpers.h"
#include "settings.h"
#include "sprites.h"
#include "logging.h"

extern GameSettings game_settings;
extern int record_mode;

void do_load_game(Enemy *autosave, int *mission, int *game_modifiers, int slot)
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
        if (*mission == 1)
            autosave->id = NO_OWNER;
}

ALLEGRO_BITMAP *menu_sprites;

void show_help()
{
    char help_path[256];
    sprintf(help_path, DATADIR "%s\\help.dat", game_settings.mission_pack);
    FILE *f = fopen(help_path, "r");
    ALLEGRO_COLOR color = al_map_rgb(255, 255, 255);
    ALLEGRO_COLOR saved_colors[10];
    memset(saved_colors, 0, sizeof(saved_colors));
    const int line_height = 16;
    int line = 0;
    int margin = 5;
    const int y_margin = 5;
    al_clear_to_color(BLACK);
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
                int r = 255, g = 255, b = 255, colorref = -1;
                int n = sscanf(s, "%*s %d %d %d %d", &r, &g, &b, &colorref);
                if (n == 1 && r >= 0 && r < 10)
                {
                    color = saved_colors[r];
                }
                else
                {
                    color = al_map_rgb(r, g, b);
                    if (colorref >= 0 && colorref < 10)
                    {
                        saved_colors[colorref] = color;
                    }
                }
            }
            if (!strcmp(cmd, "#sprite"))
            {
                int id = -1, x = 0, y = 0, dx = 0, dy = 0;
                sscanf(s, "%*s %d %d %d %d %d", &id, &x, &y, &dx, &dy);
                draw_sprite_animated(menu_sprites, id, x, y, dx, dy);
            }
            if (!strcmp(cmd, "#rect"))
            {
                int x = 0, y = 0, w = 0, h = 0, r = 0, g = 0, b = 0;
                sscanf(s, "%*s %d %d %d %d %d %d %d", &x, &y, &w, &h, &r, &g, &b);
                rectfill(x, y, x + w, y + h, al_map_rgb(r, g, b));
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
                int keys[] = {ALLEGRO_KEY_SPACE, ALLEGRO_KEY_ESCAPE};
                int key = wait_key_presses(keys, 2);
                al_clear_to_color(BLACK);
                line = 0;
                if (!strcmp(cmd, "#doc-end") || key == ALLEGRO_KEY_ESCAPE)
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

const char *game_modifiers_to_str(int game_modifiers)
{
    if (game_modifiers == 0)
        return "normal";
    if (game_modifiers == GAMEMODIFIER_BRUTAL)
        return "brutally hard";
    if (game_modifiers == GAMEMODIFIER_DOUBLED_SHOTS)
        return "explosion madness";
    if (game_modifiers == (GAMEMODIFIER_OVERPOWERED_POWERUPS | GAMEMODIFIER_OVERPRICED_POWERUPS))
        return "over power up";
    if (game_modifiers == GAMEMODIFIER_MULTIPLIED_GOLD)
        return "power up only";
    return "unknown";
}

void menu_play_sample(ALLEGRO_SAMPLE *s, ALLEGRO_SAMPLE_ID *id)
{
    if (game_settings.sfx_vol == 0)
        return;
    al_play_sample(s, game_settings.sfx_vol, 0, 1, ALLEGRO_PLAYMODE_ONCE, id);
}

struct menu_item
{
    char name[100];
    char description[3][100];
    int selectable;
};

struct menu
{
    char title[100];
    struct menu_item items[20];
    int num_items;
    int selected_item;
};

void add_menu_item(struct menu *m, const char *name, const char *description, ...)
{
    if (m->num_items >= 20)
        return;
    va_list args;
    va_start(args, description);

    char full_description[300], *start = full_description;
    vsprintf(full_description, description, args);
 
    int descr_idx = 0;
    struct menu_item *mi = &m->items[m->num_items];
    mi->selectable = 1;
    m->num_items++;
    strcpy(mi->name, name);
    char *c = start;
    int cont = 1;
    while (cont)
    {
        if (*c == '\n' || *c == 0)
        {
            cont = *c != 0;
            *c = 0;
            char *descr;
            if (descr_idx < 3)
            {
                strcpy(mi->description[descr_idx], start);
            }
            start = c + 1;
            descr_idx++;
        }
        c++;
    }
}

struct menu create_menu(const char *title, ...)
{
    va_list args;
    va_start(args, title);
    struct menu m;
    memset(&m, 0, sizeof(m));
    vsprintf(m.title, title, args);
    return m;
}

ALLEGRO_SAMPLE *menu_select = NULL;

void display_menu(struct menu *menu_state)
{   
    ALLEGRO_BITMAP *menubg = al_load_bitmap(DATADIR "\\hell.jpg");
    ALLEGRO_SAMPLE *s_c = al_load_sample(MENU_SAMPLE_FILENAME);

    const int menu_item_height = 10;
    const int menu_item_margin = 5;
    const int y_offset = 60;
    int key = 0;
    int flicker = 0;
    ALLEGRO_SAMPLE_ID id;
    while (key != ALLEGRO_KEY_ENTER)
    {
        al_draw_scaled_bitmap(menubg, 0, 0, 480, 360, 0, 0, 480 * 2, 360 * 2, 0);
        al_draw_textf(get_font(), DARK_RED, 30, 25, 0, menu_state->title);
        al_draw_textf(get_font(), RED, 28, 23, 0, menu_state->title);
        int cursor_y = y_offset;
        for (int i = 0; i < menu_state->num_items; i++)
        {
            struct menu_item *mi = &menu_state->items[i];
            al_draw_textf(get_font(), mi->selectable ? al_map_rgb(64, 64, 127) : GRAY(127), 40, cursor_y, 0, mi->name);
            if (menu_state->selected_item == i)
            {
                al_draw_filled_circle(30, cursor_y + 3, 6, al_map_rgb(200, 0, 0));
            }
            cursor_y += menu_item_height;
            for (int j = 0; j < 3; j++)
            {
                if (mi->description[j][0])
                {
                    al_draw_textf(get_font(),  mi->selectable ? al_map_rgb(127, 127, 255) : GRAY(127), 80, cursor_y,
                        0, mi->description[j]);
                    cursor_y += menu_item_height;
                }
            }
            cursor_y += menu_item_margin;
        }
        al_flip_display();
        int keys[] = {ALLEGRO_KEY_ENTER, ALLEGRO_KEY_UP, ALLEGRO_KEY_DOWN};
        key = wait_key_presses(keys, 3);
        if (key == ALLEGRO_KEY_UP && menu_state->selected_item > 0)
        {
            int si = menu_state->selected_item - 1;
            while (si >= 0 && !menu_state->items[si].selectable) si--;
            if (si >= 0)
            {
                menu_state->selected_item = si;
                menu_play_sample(s_c, &id);
            }
        }
        if (key == ALLEGRO_KEY_DOWN && menu_state->selected_item < menu_state->num_items - 1)
        {
            int si = menu_state->selected_item + 1;
            while (si < menu_state->num_items && !menu_state->items[si].selectable) si++;
            if (si < menu_state->num_items)
            {
                menu_state->selected_item = si;
                menu_play_sample(s_c, &id);
            }
        }
    }
    menu_play_sample(menu_select, &id);
    al_destroy_bitmap(menubg);
    al_destroy_sample(s_c);
}

int display_load_game_menu()
{
    struct menu m = create_menu("Load game");
    add_menu_item(&m, "Cancel", "Cancel and return to previous menu");
    for (int slot = 0; slot < 9; slot++)
    {
        char slot_name[32];
        sprintf(slot_name, "Load slot %d", slot);
        int current_slot_has_save, current_slot_mission, current_slot_game_modifiers;
        peek_into_save_data(slot, &current_slot_has_save, &current_slot_mission, &current_slot_game_modifiers);
        if (current_slot_has_save)
        {
            add_menu_item(&m, slot_name, "Mode: %s\nMission: %d",
                game_modifiers_to_str(current_slot_game_modifiers), current_slot_mission);
        }
        else
        {
            add_menu_item(&m, slot_name, "EMPTY");
            m.items[m.num_items - 1].selectable = 0;
        }
    }
    display_menu(&m);
    return m.selected_item;
}

int display_save_game_menu()
{
    struct menu m = create_menu("Save game");
    add_menu_item(&m, "Cancel", "Cancel and return to previous menu");
    for (int slot = 0; slot < 9; slot++)
    {
        char slot_name[32];
        sprintf(slot_name, "Save to slot %d", slot);
        int current_slot_has_save, current_slot_mission, current_slot_game_modifiers;
        peek_into_save_data(slot, &current_slot_has_save, &current_slot_mission, &current_slot_game_modifiers);
        if (current_slot_has_save)
        {
            add_menu_item(&m, slot_name, "Mode: %s\nMission: %d",
                game_modifiers_to_str(current_slot_game_modifiers), current_slot_mission);
        }
        else
        {
            add_menu_item(&m, slot_name, "EMPTY");
        }
    }
    display_menu(&m);
    return m.selected_item;
}

int display_game_mode_menu(int game_modifiers)
{
    const int modifiers[] = {
        0,
        GAMEMODIFIER_BRUTAL,
        GAMEMODIFIER_DOUBLED_SHOTS,
        GAMEMODIFIER_OVERPOWERED_POWERUPS | GAMEMODIFIER_OVERPRICED_POWERUPS,
        GAMEMODIFIER_MULTIPLIED_GOLD,
        -1
    };
    
    struct menu m = create_menu("Change game mode");
    
    for (int i = 0; modifiers[i] != -1; i++)
    {
        if (modifiers[i] == game_modifiers)
        {
            m.selected_item = i;
            break;
        }
    }

    add_menu_item(&m, game_modifiers_to_str(modifiers[0]), "Normal gameplay");
    add_menu_item(&m, game_modifiers_to_str(modifiers[1]),
        "- Tougher enemies                 - More enemies\n"
        "- Less souls for buing powerups   - Less effective fireball spells\n"
        "- More expensive powerups");
    add_menu_item(&m, game_modifiers_to_str(modifiers[2]),
        "Whenever anything in the game would shoot a fireball\n"
        "two fireballs are spawned instead of just one.");
    add_menu_item(&m, game_modifiers_to_str(modifiers[3]),
        "All powerups are overpowered but also very expensive");
    add_menu_item(&m, game_modifiers_to_str(modifiers[4]),
        "Player does not regenerate health or mana.\n"
        "Player has 20 souls initially at the start of each level.");
    display_menu(&m);

    return modifiers[m.selected_item];
}

int display_in_game_menu(int game_modifiers, int mission)
{
    struct menu m = create_menu("Paused -- %s -- level %d",
        game_modifiers_to_str(game_modifiers & ~GAMEMODIFIER_ARENA_FIGHT), mission);
    add_menu_item(&m, "Return", "Close the menu and continue playing");
    add_menu_item(&m, "Save game", "");
    if (game_modifiers & GAMEMODIFIER_ARENA_FIGHT)
    {
        m.items[m.num_items - 1].selectable = 0;
    }
    add_menu_item(&m, "Game options", "Change the game options");
    add_menu_item(&m, "View help", "");
    add_menu_item(&m, "Exit to main menu", "");
    display_menu(&m);
    return m.selected_item;
}

int display_new_game_menu(int game_modifiers)
{
    struct menu m = create_menu("Start new %s game", game_modifiers_to_str(game_modifiers));
    add_menu_item(&m, "Exit to main menu", "");
    add_menu_item(&m, "Story mode", "Begin a new story mode game");
    ArenaHighscore arena_highscore;
    access_arena_highscore(&arena_highscore, 1);
    for (int arena = 0; arena < game_settings.arena_config.number_of_arenas; arena++)
    {
        int kills = 0;
        for (int i = 0; i < ARENACONF_HIGHSCORE_MAP_SIZE; i++)
        {
            if (arena_highscore.mode[arena][i] == game_modifiers)
            {
                kills = arena_highscore.kills[arena][i];
                break;
            }
        }
        add_menu_item(&m, game_settings.arena_config.arenas[arena].name,
            "Arena fight! Kill as many enemies as you can!\nHighscore: %d kills", kills);
    }
    display_menu(&m);
    return m.selected_item;
}

int display_game_options(int default_opt)
{
    struct menu m = create_menu("Options");
    m.selected_item = default_opt;
    add_menu_item(&m, "Exit to main menu", "");
    add_menu_item(&m, "Set music on/off", "Current: %s", game_settings.music_on ? "on" : "off");
    add_menu_item(&m, "Next music track", "");
    add_menu_item(&m, "Set music volume", "Current: %d %%%%", (int)(100 * game_settings.music_vol));
    add_menu_item(&m, "Set sound volume", "Current: %d %%%%", (int)(100 * game_settings.sfx_vol));
    add_menu_item(&m, "Set vibration intensity", "Current: %s (%d)",
        game_settings.vibration_mode < 12 ? (game_settings.vibration_mode < 5 ? "heavy" : "medium") : "light", game_settings.vibration_mode);
    display_menu(&m);
    return m.selected_item;
}

int display_range_menu(const char *title, const char *fmt, int range_start, int count, int step)
{
    struct menu m = create_menu(title);
    for (int i = 0; i < count; i++)
    {
        int val = range_start + i * step;
        char item[100];
        sprintf(item, fmt, val);
        add_menu_item(&m, item, "");
    }
    display_menu(&m);
    return range_start + m.selected_item * step;
}

void game_option_menu()
{
    GameSettings orig;
    memcpy(&orig, &game_settings, sizeof(GameSettings));
    int choice = 0;
    do
    {
        choice = display_game_options(choice);
        if (choice == 1)
        {
            game_settings.music_on = !game_settings.music_on;
        }
        else if (choice == 2)
        {
            switch_track(get_current_track() + 1);
        }
        else if (choice == 3)
        {
            int vol = display_range_menu("Set music volume", "%d %%%%", 10, 10, 10);
            game_settings.music_vol = vol / 100.0f; 
        }
        else if (choice == 4)
        {
            int vol = display_range_menu("Set sound volume", "%d %%%%", 10, 10, 10);
            game_settings.sfx_vol = vol / 100.0f; 
        }
        else if (choice == 5)
        {
            game_settings.vibration_mode = display_range_menu("Set vibration intensity", "%d", 1, 16, 1);
        }
    } while (choice != 0);

    if (memcmp(&orig, &game_settings, sizeof(GameSettings)))
    {
        LOG("Game settings changed, saving\n");
        save_settings();
    }
}

int display_main_menu(int game_modifiers)
{
    struct menu m = create_menu("DESTINATION UNDERWORLD");
    add_menu_item(&m, "Start new game", "");
    add_menu_item(&m, "Load game", "");
    add_menu_item(&m, "Change game mode", "Current: %s", game_modifiers_to_str(game_modifiers));
    add_menu_item(&m, "Game options", "Change the game options");
    add_menu_item(&m, "View help", "");
    add_menu_item(&m, "Exit", "");
    display_menu(&m);
    return m.selected_item;
}

void load_menu_sprites()
{
    char path[256];
    get_data_filename(path, "sprites.png");
    menu_sprites = al_load_bitmap(path);
    al_convert_mask_to_alpha(menu_sprites, al_map_rgb(255, 0, 255));
}

int menu(int ingame, Enemy *autosave, int *mission, int *game_modifiers)
{
    // Make these static so that entering the menu mid-game will not forget previous choises
    static int level_set = 0;

    load_menu_sprites();
    if (!menu_select)
    {
        menu_select = al_load_sample(MENU_SELECT_SAMPLE_FILENAME);
    }

    ALLEGRO_TRANSFORM transform;
    al_identity_transform(&transform);
    al_scale_transform(&transform, 2, 2);
    al_use_transform(&transform);

    int switch_level = 1;
    ALLEGRO_SAMPLE_ID id;

    if (record_mode == RECORD_MODE_PLAYBACK)
    {
        al_clear_to_color(BLACK);
        al_draw_textf(get_font(), BLACK, SCREEN_W / 2, SCREEN_H / 2, ALLEGRO_ALIGN_CENTRE, "Press enter to start demo playback");
        wait_key_press(ALLEGRO_KEY_ENTER);
    }
    else
    {
        int exit_menu = 0, main_menu = !ingame;
        if (ingame)
        {
            while (!exit_menu)
            {
                int ingame_menu_selection = display_in_game_menu(*game_modifiers, *mission);
                switch (ingame_menu_selection)
                {
                    // Return game
                    case 0:
                        switch_level = 0;
                        exit_menu = 1;
                    break;
                    // Save game
                    case 1:
                    {
                        int save_slot = display_save_game_menu();
                        if (save_slot != 0)
                        {
                            save_game(autosave, *mission, *game_modifiers, save_slot - 1);
                        }
                    }
                    break;
                    //  Game options
                    case 2:
                    {
                        game_option_menu();
                    }
                    break;
                    // Show help
                    case 3:
                    {
                        show_help();
                    }
                    break;
                    // Exit to main menu
                    case 4:
                    {
                        main_menu = 1;
                        exit_menu = 1;
                        *game_modifiers &= ~GAMEMODIFIER_ARENA_FIGHT;
                    }
                    default:
                    break;
                }
            }
            exit_menu = 0;
        }
        while (main_menu && !exit_menu)
        {
            int main_menu_selection = display_main_menu(*game_modifiers);
            switch (main_menu_selection)
            {
                // start new game
                case 0:
                {
                    int level_set = display_new_game_menu(*game_modifiers);
                    if (level_set == 1)
                    {
                        exit_menu = 1;
                        *mission = 1;
                        autosave->id = NO_OWNER;
                    }
                    else if (level_set > 1)
                    {
                        int arena = level_set - 2;
                        exit_menu = 1;
                        *mission = game_settings.arena_config.arenas[arena].level_number;
                        *game_modifiers |= GAMEMODIFIER_ARENA_FIGHT;
                        autosave->id = NO_OWNER;
                    }
                }
                break;
                // load game
                case 1:
                {
                    int load_slot = display_load_game_menu();
                    if (load_slot != 0)
                    {
                        exit_menu = 1;
                        do_load_game(autosave, mission, game_modifiers, load_slot - 1);
                    }
                }
                break;
                // Change game mode
                case 2:
                {
                    *game_modifiers = display_game_mode_menu(*game_modifiers);
                }
                break;
                // game options
                case 3:
                {
                    game_option_menu();
                }
                break;
                // show help
                case 4:
                {
                    show_help();
                }
                break;
                // exit
                case 5:
                {
                    *mission = 0;
                    exit_menu = 1;
                }
                break;
                default:
                break;
            }
        }
    }
    al_destroy_bitmap(menu_sprites);
    if (*mission == 0)
    {
        al_destroy_sample(menu_select);
        menu_select = NULL;
    }

    return switch_level;
}
