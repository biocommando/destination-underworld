#include <math.h>
#include <stdarg.h>
#include "menu.h"
#include "gamePersistence.h"
#include "duColors.h"
#include "helpers.h"
#include "settings.h"
#include "sprites.h"
#include "logging.h"
#include "sampleRegister.h"
#include "midi_playback.h"
#include "record_file.h"
#include "sha1/du_dmac.h"

static void do_load_game(Enemy *autosave, int *mission, int *game_modifiers, int slot)
{
    load_game(autosave, mission, game_modifiers, slot);
    autosave->alive = 1;
    autosave->killed = 0;
    if (*mission == 1)
        autosave->alive = 0;
}

static ALLEGRO_BITMAP *menu_sprites;

static void show_help()
{
    char help_path[256];
    sprintf(help_path, DATADIR "%s\\help.dat", get_game_settings()->mission_pack);
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
                al_draw_filled_rectangle(x, y, x + w + 1, y + h + 1, al_map_rgb(r, g, b));
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

static const char *game_modifiers_to_str(int game_modifiers)
{
    if (game_modifiers == 0)
        return "Normal";
    if (game_modifiers == GAMEMODIFIER_BRUTAL)
        return "Brutally hard";
    if (game_modifiers == GAMEMODIFIER_DOUBLED_SHOTS)
        return "Explosion madness";
    if (game_modifiers == (GAMEMODIFIER_OVERPOWERED_POWERUPS | GAMEMODIFIER_OVERPRICED_POWERUPS))
        return "Over power up";
    if (game_modifiers == GAMEMODIFIER_MULTIPLIED_GOLD)
        return "Power up only";
    if (game_modifiers == (GAMEMODIFIER_POTION_ON_DEATH | GAMEMODIFIER_MULTIPLIED_GOLD | GAMEMODIFIER_NO_GOLD))
        return "Potions only";
    if (game_modifiers == (GAMEMODIFIER_UBER_WIZARD | GAMEMODIFIER_NO_GOLD | GAMEMODIFIER_MULTIPLIED_GOLD))
        return "Uber wizard";
    return "unknown";
}

static void menu_play_sample(int sample_id)
{
    if (get_game_settings()->sfx_vol == 0)
        return;
    reset_sample_triggers();
    trigger_sample(sample_id, 255);
}

struct menu_item
{
    char name[100];
    char description[3][100];
    int selectable;
    int item_id;
    int meta;
};

static int get_menu_item_id(const char *id_str)
{
    int id = 0;
    for (int i = 0; id_str[i] && i < 4; i++)
    {
        id = id << 8;
        id = id | id_str[i];
    }
    return id;
}

struct menu
{
    char title[100];
    struct menu_item items[20];
    int num_items;
    int selected_item;
    int cancel_menu_item_id;
};

static void set_item_by_id(struct menu *m, int id)
{
    for (int i = 0; i < m->num_items; i++)
    {
        if (m->items[i].item_id == id)
        {
            m->selected_item = i;
            return;
        }
    }
}

static struct menu_item *add_menu_item(struct menu *m, const char *name, const char *description, ...)
{
    if (m->num_items >= 20)
        return m->items;
    va_list args;
    va_start(args, description);

    char full_description[300], *start = full_description;
    vsprintf(full_description, description, args);

    int descr_idx = 0;
    struct menu_item *mi = &m->items[m->num_items];
    mi->meta = 0;
    mi->selectable = 1;
    m->num_items++;
    strcpy(mi->name, name);
    mi->item_id = get_menu_item_id(name);
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
    return mi;
}

#define MENU_ID_CANCEL_OPTION_NOT_SET 0x1234FFFF

struct menu create_menu(const char *title, ...)
{
    va_list args;
    va_start(args, title);
    struct menu m;
    memset(&m, 0, sizeof(m));
    vsprintf(m.title, title, args);
    m.cancel_menu_item_id = MENU_ID_CANCEL_OPTION_NOT_SET;
    return m;
}

static void display_menu(struct menu *menu_state)
{
    while (!menu_state->items[menu_state->selected_item].selectable)
    {
        menu_state->selected_item = (menu_state->selected_item + 1) % menu_state->num_items;
    }
    ALLEGRO_BITMAP *menubg = al_load_bitmap(DATADIR "\\hell.jpg");

    const int font_height = al_get_font_line_height(get_menu_font());
    const int menu_item_height = font_height + 2;
    const int small_menu_item_height = al_get_font_line_height(get_font());
    const int menu_item_margin = 3;
    const int y_offset = 60;
    int timer = 0, select_animation_timer = 0;
    wait_key_release(ALLEGRO_KEY_ESCAPE);
    wait_key_release(ALLEGRO_KEY_ENTER);
    int key_pressed = 0, key_released = 0;
    while (1)
    {
        timer++;
        al_draw_scaled_bitmap(menubg, 0, 0, 480, 360, 0, 0, 480 * 2, 360 * 2, 0);
        al_draw_textf(get_menu_title_font(), DARK_RED, 30, 25, 0, menu_state->title);
        al_draw_textf(get_menu_title_font(), RED, 28, 23, 0, menu_state->title);
        int cursor_y = y_offset;
        for (int i = 0; i < menu_state->num_items; i++)
        {
            struct menu_item *mi = &menu_state->items[i];
            int cursor_y_offset = 0;
            if (menu_state->selected_item == i)
            {
                cursor_y_offset = sin(timer * 0.3) * 3;
                al_draw_filled_circle(30, cursor_y + font_height / 2, 5 + sin(timer * 0.1) * 2, al_map_rgb(150 + sin(timer * 0.1) * 50, 0, 0));
            }
            if (!mi->meta)
            {
                ALLEGRO_COLOR col = mi->selectable ? al_map_rgb(64, 64, 127) : GRAY(127);
                if (select_animation_timer > 0 && menu_state->selected_item == i)
                    col = GRAY(128 + ((select_animation_timer / 2) % 2) * 127);
                al_draw_textf(get_menu_font(), col, 40, cursor_y + cursor_y_offset, 0, mi->name);
                cursor_y += menu_item_height;
            }
            else
            {
                cursor_y += menu_item_height / 2;
            }
            for (int j = 0; j < 3; j++)
            {
                if (mi->description[j][0])
                {
                    ALLEGRO_COLOR col = mi->selectable ? al_map_rgb(127, 127, 255) : GRAY(127);
                    if (mi->meta)
                        col = al_map_rgb(182, 182, 0);
                    al_draw_textf(get_font(), col, mi->meta ? 30 : 80, cursor_y, 0, mi->description[j]);
                    cursor_y += small_menu_item_height;
                }
            }
            if (mi->meta)
                cursor_y += menu_item_height / 2;
            else
                cursor_y += menu_item_margin;
        }
        al_flip_display();
        wait_delay_ms(30);
        if (select_animation_timer > 0)
        {
            select_animation_timer--;
            if (select_animation_timer == 0)
                break;
            continue;
        }

        int keys[] = {ALLEGRO_KEY_ENTER, ALLEGRO_KEY_UP, ALLEGRO_KEY_DOWN, ALLEGRO_KEY_ESCAPE};
        key_released = 0;
        if (key_pressed && !check_key(key_pressed))
        {
            key_released = key_pressed;
            key_pressed = 0;
        }
        else
        {
            int key = check_keys(keys, sizeof(keys) / sizeof(int));
            if (key)
                key_pressed = key;
        }
        if (key_released == ALLEGRO_KEY_UP)
        {
            int initial_item = menu_state->selected_item - 1;
            do
            {
                int si = initial_item;
                while (si >= 0 && !menu_state->items[si].selectable)
                    si--;
                if (si >= 0)
                {
                    menu_state->selected_item = si;
                    menu_play_sample(SAMPLE_MENU_CHANGE);
                    initial_item = 999;
                }
                else
                {
                    initial_item = menu_state->num_items - 1;
                }
            } while (initial_item != 999);
        }
        if (key_released == ALLEGRO_KEY_DOWN)
        {
            int initial_item = menu_state->selected_item + 1;
            do
            {
                int si = initial_item;
                while (si < menu_state->num_items && !menu_state->items[si].selectable)
                    si++;
                if (si < menu_state->num_items)
                {
                    menu_state->selected_item = si;
                    menu_play_sample(SAMPLE_MENU_CHANGE);
                    initial_item = 999;
                }
                else
                {
                    initial_item = 0;
                }
            } while (initial_item != 999);
        }
        if (key_released == ALLEGRO_KEY_ESCAPE && menu_state->cancel_menu_item_id != MENU_ID_CANCEL_OPTION_NOT_SET)
        {
            menu_play_sample(SAMPLE_MENU_SELECT);
            set_item_by_id(menu_state, menu_state->cancel_menu_item_id);
            break;
        }
        if (key_released == ALLEGRO_KEY_ENTER)
        {
            menu_play_sample(SAMPLE_MENU_SELECT);
            select_animation_timer = 10;
        }
    }
    al_destroy_bitmap(menubg);
}

static int display_load_game_menu()
{
    struct menu m = create_menu("Load game");
    m.cancel_menu_item_id = add_menu_item(&m, "Cancel", "Cancel and return to previous menu")->item_id;
    for (int slot = 0; slot < 9; slot++)
    {
        char slot_name[32];
        sprintf(slot_name, "Load slot %d", slot);
        int current_slot_has_save, current_slot_mission, current_slot_game_modifiers;
        peek_into_save_data(slot, &current_slot_has_save, &current_slot_mission, &current_slot_game_modifiers);
        if (current_slot_has_save)
        {
            add_menu_item(&m, slot_name, "Mode: %s\nLevel: %d",
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

static int display_save_game_menu()
{
    struct menu m = create_menu("Save game");
    struct menu_item *mi = add_menu_item(&m, "Cancel", "Cancel and return to previous menu");
    m.cancel_menu_item_id = mi->item_id;
    for (int slot = 0; slot < 9; slot++)
    {
        char slot_name[32];
        sprintf(slot_name, "Save to slot %d", slot);
        int current_slot_has_save, current_slot_mission, current_slot_game_modifiers;
        peek_into_save_data(slot, &current_slot_has_save, &current_slot_mission, &current_slot_game_modifiers);
        if (current_slot_has_save)
        {
            add_menu_item(&m, slot_name, "Mode: %s\nLevel: %d",
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

static int check_game_modes_beaten(const int *modifiers, int sz)
{
    if (!get_game_settings()->require_authentication)
        return 0;

    LOG("check_game_modes_beaten\n");
    dmac_sha1_set_ctx(AUTH_CTX_COMPLETED_GAME_MODES);

    char path[256];
    get_data_filename(path, "completed-game-modes.dat");

    int beaten_modes = 0;
    for (int i = 0; i < sz; i++)
    {
        int mode = modifiers[i];
        char id[20];
        sprintf(id, "game_modifiers_%d", mode);
        char key[101], hash_hex[DMAC_SHA1_HASH_SIZE * 2 + 1];
        if (record_file_scanf(path, id, "%*s %100s %40s", key, hash_hex) != 2)
        {
            LOG("Record not found for %s\n", game_modifiers_to_str(mode));
            continue;
        }
        char check_str[200];
        sprintf(check_str, "%s %s ", id, key);

        char hash[DMAC_SHA1_HASH_SIZE];
        convert_sha1_hex_to_hash(hash, hash_hex);
        if (dmac_sha1_verify_string(check_str, strlen(check_str), hash) != 0)
        {
            LOG("DMAC hash verification failed for %s\n", game_modifiers_to_str(mode));
            continue;
        }
        LOG("game mode %s beaten\n", game_modifiers_to_str(mode));
        beaten_modes++;
    }
    return beaten_modes == sz;
}

static int display_game_mode_menu(int game_modifiers)
{
    const int modifiers[] = {
        0,
        GAMEMODIFIER_BRUTAL,
        GAMEMODIFIER_DOUBLED_SHOTS,
        GAMEMODIFIER_OVERPOWERED_POWERUPS | GAMEMODIFIER_OVERPRICED_POWERUPS,
        GAMEMODIFIER_MULTIPLIED_GOLD,
        (GAMEMODIFIER_POTION_ON_DEATH | GAMEMODIFIER_NO_GOLD | GAMEMODIFIER_MULTIPLIED_GOLD),
        (GAMEMODIFIER_UBER_WIZARD | GAMEMODIFIER_NO_GOLD | GAMEMODIFIER_MULTIPLIED_GOLD),
        -1};

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
                  "- More and tougher enemies\n"
                  "- Less souls for buying powerups\n"
                  "- More expensive powerups and less effective fireball spells");
    add_menu_item(&m, game_modifiers_to_str(modifiers[2]),
                  "Whenever anything in the game would shoot a fireball\n"
                  "two fireballs are spawned instead of just one.");
    add_menu_item(&m, game_modifiers_to_str(modifiers[3]),
                  "All powerups are overpowered but also very expensive");
    add_menu_item(&m, game_modifiers_to_str(modifiers[4]),
                  "Player does not regenerate health or mana.\n"
                  "Player has 20 souls initially at the start of each level.");
    add_menu_item(&m, game_modifiers_to_str(modifiers[5]),
                  "Player does not regenerate health or mana.\n"
                  "Player does not have access to powerups.\n"
                  "Player uses potions to survive.");
    if (check_game_modes_beaten(modifiers, 6))
    {
        add_menu_item(&m, game_modifiers_to_str(modifiers[6]), "Player does not have access to powerups\n"
                                         "but the base weapon is super powerful with multiple modes\n"
                                         "that can be switched using X and L, or using the powerup keys.");
    }
    else
    {
        struct menu_item *mi = add_menu_item(&m, "????",
                                             "Beat the game in all other game modes to unlock the secret game mode.");
        mi->selectable = 0;
    }

    m.cancel_menu_item_id = m.items[m.selected_item].item_id;

    display_menu(&m);

    return modifiers[m.selected_item];
}

static int display_in_game_menu(int game_modifiers, int mission)
{
    struct menu m = create_menu("Paused -- %s -- level %d",
                                game_modifiers_to_str(game_modifiers & ~GAMEMODIFIER_ARENA_FIGHT), mission);
    struct menu_item *mi = add_menu_item(&m, "Return", "Close the menu and continue playing");
    m.cancel_menu_item_id = mi->item_id;
    add_menu_item(&m, "Perks", "Spend experience points for upgrades!");
    add_menu_item(&m, "Save game", "");
    if (game_modifiers & GAMEMODIFIER_ARENA_FIGHT)
    {
        m.items[m.num_items - 2].selectable = 0;
        m.items[m.num_items - 1].selectable = 0;
    }
    add_menu_item(&m, "Game options", "Change the game options");
    add_menu_item(&m, "View help", "");
    add_menu_item(&m, "Exit to main menu", "Abandon current game and lose all progress");
    display_menu(&m);
    return m.items[m.selected_item].item_id;
}

static int display_new_game_menu(int game_modifiers)
{
    struct menu m = create_menu("Start new %s game", game_modifiers_to_str(game_modifiers));
    struct menu_item *mi = add_menu_item(&m, "Exit to main menu", "");
    m.cancel_menu_item_id = mi->item_id;
    add_menu_item(&m, "Change game mode", "Current: %s", game_modifiers_to_str(game_modifiers));
    add_menu_item(&m, "Story mode", "Begin a new story mode game");
    ArenaHighscore arena_highscore;
    access_arena_highscore(&arena_highscore, 1);
    mi = add_menu_item(&m, "(meta)", "The levels below are arena fights where you need to kill\nas many enemies as you can before dying yourself.");
    mi->meta = 1;
    mi->selectable = 0;
    for (int arena = 0; arena < get_game_settings()->arena_config.number_of_arenas; arena++)
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
        mi = add_menu_item(&m, get_game_settings()->arena_config.arenas[arena].name,
                           "Highscore: %d kills", kills);
        mi->item_id = arena;
    }
    set_item_by_id(&m, get_menu_item_id("Story"));
    display_menu(&m);
    return m.items[m.selected_item].item_id;
}

static const char *vibration_intensity_fmt(int i);

static int display_game_options(int default_opt)
{
    struct menu m = create_menu("Options");
    struct menu_item *mi;
    mi = add_menu_item(&m, "Exit to main menu", "");
    m.cancel_menu_item_id = mi->item_id;
    mi = add_menu_item(&m, "Set music on/off", "Current: %s", get_game_settings()->music_on ? "on" : "off");
    mi->item_id = get_menu_item_id("m.on/off");
    add_menu_item(&m, "Select music track", "Now playing: %s", get_midi_playlist_entry_file_name(-1));
    mi = add_menu_item(&m, "Set music volume", "Current: %d %%%%", (int)(100 * get_game_settings()->music_vol));
    mi->item_id = get_menu_item_id("m.vol");
    mi = add_menu_item(&m, "Set sound volume", "Current: %d %%%%", (int)(100 * get_game_settings()->sfx_vol));
    mi->item_id = get_menu_item_id("s.vol");
    char vibr_intensity_text[100];
    sprintf(vibr_intensity_text, vibration_intensity_fmt(get_game_settings()->vibration_mode), get_game_settings()->vibration_mode);
    mi = add_menu_item(&m, "Set vibration intensity", "Current: %s", vibr_intensity_text);
    mi->item_id = get_menu_item_id("vibrations");
    mi = add_menu_item(&m, "Window mode", "Current: %s\nThe game needs to be restarted to take new window mode into use.", get_game_settings()->fullscreen ? "Full screen" : "Windowed");
    mi->item_id = get_menu_item_id("Window");
    set_item_by_id(&m, default_opt);
    display_menu(&m);
    return m.items[m.selected_item].item_id;
}

static int display_range_menu(const char *title, const char *(*fmt)(int), int range_start, int count, int step, int default_opt)
{
    struct menu m = create_menu(title);
    for (int i = 0; i < count; i++)
    {
        int val = range_start + i * step;
        char item[100];
        sprintf(item, fmt(val), val);
        add_menu_item(&m, item, "");
    }
    m.selected_item = (default_opt - range_start) / step;
    if (m.selected_item >= m.num_items || m.selected_item == 0)
        m.selected_item = 0;
    m.cancel_menu_item_id = m.items[m.selected_item].item_id;
    display_menu(&m);
    return range_start + m.selected_item * step;
}

static int display_select_track_menu()
{
    struct menu m = create_menu("Select track");
    const char *fname;
    const char *current = get_midi_playlist_entry_file_name(-1);
    for (int i = 0; fname = get_midi_playlist_entry_file_name(i); i++)
    {
        add_menu_item(&m, fname, "");
        if (!strcmp(current, fname))
            m.selected_item = i;
    }
    add_menu_item(&m, "Randomize", "");
    int randomize_idx = m.num_items - 1;
    add_menu_item(&m, "Cancel", "");
    int cancel_idx = m.num_items - 1;
    m.items[cancel_idx].item_id = 1;
    m.cancel_menu_item_id = 1;
    display_menu(&m);
    if (m.selected_item == randomize_idx)
        return -2;
    return m.selected_item == cancel_idx ? -1 : m.selected_item;
}

static const char *percent_fmt(int i)
{
    return "%d %%%%";
}

static const char *vibration_intensity_fmt(int i)
{
    if (i == 0)
        return "Off";
    if (i < 5)
        return "%2d -- Heavy";
    if (i < 12)
        return "%2d -- Medium";
    return "%2d -- Light";
}

static void game_option_menu()
{
    GameSettings orig;
    memcpy(&orig, get_game_settings(), sizeof(GameSettings));
    int choice = 0;
    do
    {
        choice = display_game_options(choice);
        if (choice == get_menu_item_id("m.on/off"))
        {
            get_game_settings()->music_on = !get_game_settings()->music_on;
            if (get_game_settings()->music_on)
                next_midi_track(0);
        }
        else if (choice == get_menu_item_id("Select"))
        {
            // switch_track(get_current_track() + 1);
            // next_midi_track(-1);
            int track = display_select_track_menu();
            if (track == -2)
            {
                randomize_midi_playlist();
                track = 0;
            }
            if (track >= 0)
                next_midi_track(track);
        }
        else if (choice == get_menu_item_id("m.vol"))
        {
            int default_opt = (int)(get_game_settings()->music_vol * 100 + 0.5f);
            int vol = display_range_menu("Set music volume", percent_fmt, 10, 10, 10, default_opt);
            get_game_settings()->music_vol = vol / 100.0f;
        }
        else if (choice == get_menu_item_id("s.vol"))
        {
            int default_opt = (int)(get_game_settings()->sfx_vol * 100 + 0.5f);
            int vol = display_range_menu("Set sound volume", percent_fmt, 10, 10, 10, default_opt);
            get_game_settings()->sfx_vol = vol / 100.0f;
        }
        else if (choice == get_menu_item_id("vibrations"))
        {
            get_game_settings()->vibration_mode = display_range_menu("Set vibration intensity", vibration_intensity_fmt, 0, 17, 1, get_game_settings()->vibration_mode);
        }
        else if (choice == get_menu_item_id("Window"))
        {
            get_game_settings()->fullscreen = !get_game_settings()->fullscreen;
        }
    } while (choice != get_menu_item_id("Exit"));

    if (memcmp(&orig, get_game_settings(), sizeof(GameSettings)))
    {
        LOG("Game settings changed, saving\n");
        save_settings();
    }
}

static int display_main_menu(int game_modifiers)
{
    struct menu m = create_menu("DESTINATION UNDERWORLD");
    add_menu_item(&m, "Start new game", "");
    add_menu_item(&m, "Load game", "");
    add_menu_item(&m, "Game options", "Change the game options");
    add_menu_item(&m, "View help", "");
    add_menu_item(&m, "Exit", "");
    display_menu(&m);
    return m.items[m.selected_item].item_id;
}

static void load_menu_sprites()
{
    char path[256];
    get_data_filename(path, "sprites.png");
    menu_sprites = al_load_bitmap(path);
    al_convert_mask_to_alpha(menu_sprites, al_map_rgb(255, 0, 255));
}

static void add_perk_menu_item(struct menu *m, int perks, int enough_xp, int perk_flag, const char *name, const char *description)
{
    struct menu_item *mi = add_menu_item(m, name, "%s%s", description, (perks & perk_flag) ? "\nALREADY UPGRADED" : "");
    mi->selectable = (perks & perk_flag) == 0 && enough_xp;
    mi->item_id = perk_flag;
}

static void display_perk_menu(Enemy *player)
{
    int required_xp = calculate_next_perk_xp(player->perks);
    int enough_xp = required_xp <= player->xp;
    struct menu m = create_menu("Perks");
    m.cancel_menu_item_id = 123;
    struct menu_item *mi = add_menu_item(&m, "(meta)", "Available XP: %d\nNext upgrade cost: %d", player->xp, required_xp);
    mi->meta = 1;
    mi->selectable = 0;
    add_menu_item(&m, "Return", "");
    add_perk_menu_item(&m, player->perks, enough_xp, PERK_INCREASE_MAX_HEALTH, "Iron skin", "Increases maximum health by one.");
    add_perk_menu_item(&m, player->perks, enough_xp, PERK_IMPROVE_HEALTH_POWERUP, "Healer", "Improves the Heal powerup.\n+1 health per use.");
    add_perk_menu_item(&m, player->perks, enough_xp, PERK_IMPROVE_SHIELD_POWERUP, "Protector", "Improves the Shield powerup.\nGuards from 3 hits instead of 1.");
    add_perk_menu_item(&m, player->perks, enough_xp, PERK_IMPROVE_TURRET_POWERUP, "Engineer", "Improves the Turret powerup.\nThe spawned turret explodes after shooting.");
    add_perk_menu_item(&m, player->perks, enough_xp, PERK_IMPROVE_BLAST_POWERUP, "Annihilator", "Improves the Torrent of Fire powerup.\nMakes the projectile shoot fireballs while traveling\nand spawn a turret when it explodes.");
    add_perk_menu_item(&m, player->perks, enough_xp, PERK_START_WITH_SPEED_POTION, "Speedrunner", "Start level with Potion of Gotta Go Fast.");
    add_perk_menu_item(&m, player->perks, enough_xp, PERK_START_WITH_SHIELD_POWERUP, "Paranoid", "Start level with Shield powerup active.");
    display_menu(&m);
    int selected_id = m.items[m.selected_item].item_id;
    if (selected_id == m.cancel_menu_item_id || selected_id == get_menu_item_id("Return"))
        return;
    player->xp -= required_xp;
    player->perks |= selected_id;
}

static int display_exit_to_main_menu_menu()
{
    struct menu m = create_menu("Exit to main menu?");
    m.cancel_menu_item_id = 123;

    struct menu_item *mi = add_menu_item(&m, "(meta)", "You will lose all progress. Are you sure?");
    mi->meta = 1;
    mi->selectable = 0;

    add_menu_item(&m, "No", "Return to in-game menu");
    add_menu_item(&m, "Yes", "Abandon game and go to main menu");
    add_menu_item(&m, "Exit game", "Exit the game completely");

    display_menu(&m);

    if (m.items[m.selected_item].item_id == get_menu_item_id("Yes"))
        return 1;
    if (m.items[m.selected_item].item_id == get_menu_item_id("Exit"))
        return 2;
    return 0;
}

int menu(int ingame, GlobalGameState *ggs)
{
    // Make these static so that entering the menu mid-game will not forget previous choises
    static int level_set = 0;

    load_menu_sprites();

    ALLEGRO_TRANSFORM transform;
    al_identity_transform(&transform);
    al_scale_transform(&transform, 2, 2);
    al_use_transform(&transform);

    int switch_level = 1;
    ALLEGRO_SAMPLE_ID id;

    int exit_menu = 0, main_menu = !ingame;
    if (ingame)
    {
        while (!exit_menu)
        {
            int ingame_menu_selection = display_in_game_menu(ggs->game_modifiers, ggs->mission);
            // Return game
            if (ingame_menu_selection == get_menu_item_id("Return"))
            {
                switch_level = 0;
                exit_menu = 1;
            }
            else if (ingame_menu_selection == get_menu_item_id("Perks"))
            {
                display_perk_menu(ggs->player);
            }
            else if (ingame_menu_selection == get_menu_item_id("Save"))
            {
                int save_slot = display_save_game_menu();
                if (save_slot != 0)
                {
                    save_game(&ggs->plrautosave, ggs->mission, ggs->game_modifiers, save_slot - 1);
                }
            }
            else if (ingame_menu_selection == get_menu_item_id("Game options"))
            {
                game_option_menu();
            }
            else if (ingame_menu_selection == get_menu_item_id("View help"))
            {
                show_help();
            }
            else if (ingame_menu_selection == get_menu_item_id("Exit"))
            {
                int choice = display_exit_to_main_menu_menu();
                if (choice)
                {
                    main_menu = choice == 1;
                    exit_menu = 1;
                    ggs->game_modifiers &= ~GAMEMODIFIER_ARENA_FIGHT;
                    if (choice == 2)
                        ggs->mission = 0;
                }
            }
        }
        exit_menu = 0;
    }
    while (main_menu && !exit_menu)
    {
        int main_menu_selection = display_main_menu(ggs->game_modifiers);
        if (main_menu_selection == get_menu_item_id("Start"))
        {
            int choice = 0;
            while (choice != get_menu_item_id("Exit") && !exit_menu)
            {
                choice = display_new_game_menu(ggs->game_modifiers & ~GAMEMODIFIER_ARENA_FIGHT);

                if (choice == get_menu_item_id("Change game mode"))
                {
                    ggs->game_modifiers = display_game_mode_menu(ggs->game_modifiers & ~GAMEMODIFIER_ARENA_FIGHT);
                }
                else if (choice == get_menu_item_id("Story"))
                {
                    exit_menu = 1;
                    ggs->mission = 1;
                    ggs->plrautosave.alive = 0;
                    ggs->game_modifiers &= ~GAMEMODIFIER_ARENA_FIGHT;
                }
                else if (choice < ARENACONF_MAX_NUMBER_OF_ARENAS)
                {
                    int arena = choice;
                    exit_menu = 1;
                    ggs->mission = get_game_settings()->arena_config.arenas[arena].level_number;
                    ggs->game_modifiers |= GAMEMODIFIER_ARENA_FIGHT;
                    ggs->plrautosave.alive = 0;
                }
            }
        }
        if (main_menu_selection == get_menu_item_id("Load"))
        {
            int load_slot = display_load_game_menu();
            if (load_slot != 0)
            {
                exit_menu = 1;
                do_load_game(&ggs->plrautosave, &ggs->mission, &ggs->game_modifiers, load_slot - 1);
            }
        }
        if (main_menu_selection == get_menu_item_id("Game options"))
        {
            game_option_menu();
        }
        if (main_menu_selection == get_menu_item_id("View help"))
        {
            show_help();
        }
        if (main_menu_selection == get_menu_item_id("Exit"))
        {
            ggs->mission = 0;
            exit_menu = 1;
        }
    }
    al_destroy_bitmap(menu_sprites);

    return switch_level;
}
