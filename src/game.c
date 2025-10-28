#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "logging.h"
#include "duConstants.h"
#include "duColors.h"
#include "world.h"
#include "worldInteraction.h"
#include "renderWorld.h"
#include "gamePersistence.h"
#include "menu.h"
#include "bossfightconf.h"
#include "predictableRandom.h"
#include "settings.h"
#include "keyhandling.h"
#include "sampleRegister.h"
#include "helpers.h"
#include "sprites.h"
#include "record_file.h"
#include "game_playback.h"
#include "gen_version_info.h"
#include "midi_playback.h"
#include "game.h"
#include "potion_logic.h"
#include "enemy_logic.h"
#include "boss_logic.h"
#include "bullet_logic.h"
#include "read_level.h"
#include "vfx.h"
#include "sha1/du_dmac.h"
#include "screenshot.h"
#include "game_tuning.h"

static void set_game_mode_beaten_flag(int game_modifiers)
{
    if (!get_game_settings()->require_authentication)
        return;

    char path[256];
    get_data_filename(path, "completed-game-modes.dat");

    char id[20];
    sprintf(id, "game_modifiers_%d", game_modifiers);

    char key[101];
    key[100] = 0;
    for (int i = 0; i < 100; i++)
    {
        key[i] = 'A' + (rand() + time(NULL)) % 26;
    }
    char data[200];
    sprintf(data, "%s %s ", id, key);
    char hash[DMAC_SHA1_HASH_SIZE];
    dmac_sha1_set_ctx(AUTH_CTX_COMPLETED_GAME_MODES);
    dmac_sha1_calculate_hash(hash, data, strlen(data));
    char hash_hex[DMAC_SHA1_HASH_SIZE * 2 + 1];
    convert_sha1_hash_to_hex(hash_hex, hash);

    record_file_find_and_modify(path, id);
    record_file_add_param(key);
    record_file_add_param(hash_hex);
}

static void display_arena_fight_end_screen(const World *world, GlobalGameState *ggs, const int *record_mode, int no_player_damage)
{
    int highscore_kills = get_arena_highscore(ggs->mission, ggs->game_modifiers);
    int offx = (DISPLAY_W - 340) / 2, offy = (DISPLAY_H - 125) / 2;
    for (int grayscale = 0; grayscale < 5; grayscale++)
    {
        al_draw_filled_rectangle(offx + grayscale, offy + grayscale, offx + 345 - grayscale + 1, offy + 130 - grayscale + 1, GRAY(255 - grayscale * 40));
    }
    al_draw_filled_rectangle(offx + 5, offy + 5, offx + 340 + 1, offy + 125 + 1, GRAY(60));
    al_draw_textf(get_font(), WHITE, offx + 10, offy + 10, ALLEGRO_ALIGN_LEFT, "Arena fight over, your kill count: %d", world->kills);
    if (*record_mode == RECORD_MODE_NONE && highscore_kills < world->kills)
    {
        al_draw_textf(get_font(), WHITE, offx + 10, offy + 30, ALLEGRO_ALIGN_LEFT, "Previous highscore: %d", highscore_kills);
        al_draw_textf(get_font(), WHITE, offx + 10, offy + 50, ALLEGRO_ALIGN_LEFT, "NEW HIGHSCORE!");
        if (!no_player_damage)
        {
            set_arena_highscore(ggs->mission, ggs->game_modifiers, world->kills);
        }
        else
        {
            LOG("Not saving highscore, no damage mode active");
        }
    }
    else
    {
        al_draw_textf(get_font(), WHITE, offx + 10, offy + 30, ALLEGRO_ALIGN_LEFT, "Highscore: %d", highscore_kills);
    }
    al_draw_textf(get_font(), WHITE, offx + 10, offy + 100, ALLEGRO_ALIGN_LEFT, "ENTER = replay, ESC = go back to menu");
    al_flip_display();
    int wait_keys[] = {ALLEGRO_KEY_ENTER, ALLEGRO_KEY_ESCAPE};
    if (!ggs->no_player_interaction)
    {
        int key = wait_key_presses(wait_keys, 2);
        if (key == ALLEGRO_KEY_ESCAPE && *record_mode != RECORD_MODE_PLAYBACK)
        {
            // Break out of the loop in main.c
            ggs->mission = -1;
        }
    }
}

long init_playback(World *world, GlobalGameState *ggs, int record_mode)
{
    // For playback recording filenames
    static int fname_counter = 0;
    if (record_mode == RECORD_MODE_RECORD)
    {
        char record_input_filename[256];
        sprintf(record_input_filename, "recorded-mission%d-take%d.dat", ggs->mission, ++fname_counter);
        remove(record_input_filename);

        game_playback_set_filename(record_input_filename);
        game_playback_init();
        save_game_save_data(record_input_filename, &world->plr, ggs->mission, ggs->game_modifiers, 0);
    }
    else if (record_mode == RECORD_MODE_PLAYBACK)
    {
        game_playback_init();
        load_game_save_data(game_playback_get_filename(), &world->plr, &ggs->mission, &ggs->game_modifiers, 0);
        return game_playback_get_time_stamp();
    }
    return 0;
}

static inline int calc_vibrations(int vibrations)
{
    if (vibrations > MAX_VIBRATIONS)
        vibrations = MAX_VIBRATIONS;
    if (get_game_settings()->vibration_mode != 1)
    {
        if (get_game_settings()->vibration_mode == 0)
        {
            vibrations = 0;
        }
        else
        {
            vibrations /= get_game_settings()->vibration_mode;
        }
    }
    return vibrations;
}

static inline void move_and_reload_plr(World *world)
{
    int plr_speed = get_plr_speed(world);
    enemy_reload(&world->plr, plr_speed);
    for (; plr_speed > 0; plr_speed--)
    {
        move_enemy(&world->plr, world);
    }
}

static inline void check_perks_changed(World *world, int old_perks, int *next_perk_xp)
{
    if (world->plr.perks != old_perks)
    {
        *next_perk_xp = calculate_next_perk_xp(world->plr.perks);
        if ((world->plr.perks & PERK_START_WITH_SHIELD_POWERUP) && !(old_perks & PERK_START_WITH_SHIELD_POWERUP) && world->powerups.rune_of_protection_active <= 0)
        {
            world->powerups.rune_of_protection_active = 1;
        }
        if ((world->plr.perks & PERK_START_WITH_SPEED_POTION) && !(old_perks & PERK_START_WITH_SPEED_POTION))
        {
            spawn_potion(world->plr.x, world->plr.y, POTION_ID_FAST, world->plr.roomid, world, 0);
        }
    }
}

static void write_recording_complete_state_file(World *world, GlobalGameState *ggs, long time_stamp)
{
    FILE *f = fopen(DATADIR "recording--level-complete-state.dat", "w");

    fprintf(f, "Recording complete\n");
    fprintf(f, "Mission %d, mode %d\n", ggs->mission, ggs->game_modifiers);
    fprintf(f, "Kills %d\n", world->kills);
    fprintf(f, "Time %ld\n", time_stamp);
    fprintf(f, "Enemy states\n");
    int i = 0;
    Enemy *e = &world->plr;
    fprintf(f, "player: alive %d killed %d position %d,%d health %d ammo %d roomid %d rate %d shots %d turret %d gold %d\n",
            e->alive, e->killed, e->x, e->y, e->health, e->ammo, e->roomid, e->rate, e->shots, e->turret, e->gold);
    LINKED_LIST_FOR_EACH(&world->enm, Enemy, e, 0)
    {
        fprintf(f, "enemy #%d: alive %d killed %d position %d,%d health %d ammo %d roomid %d rate %d shots %d turret %d gold %d\n",
                i++, e->alive, e->killed, e->x, e->y, e->health, e->ammo, e->roomid, e->rate, e->shots, e->turret, e->gold);
    }
    struct killed_enemy_stats *kes;
    fprintf(f, "Dead\n");
    LINKED_LIST_FOR_EACH(&world->killed_enemy_stats, struct killed_enemy_stats, kes, 0)
    {
        fprintf(f, "enemy #%d: position %d,%d ammo %d roomid %d rate %d shots %d turret %d gold %d\n",
                i++, kes->x, kes->y, kes->ammo, kes->roomid, kes->rate, kes->shots, kes->turret, kes->gold);
    }
    fprintf(f, "Potion states\n");
    Potion *p;
    i = 0;
    LINKED_LIST_FOR_EACH(&world->potions, Potion, p, 0)
    {
        fprintf(f, "potion #%d: position %d,%d roomid %d effects 0x%x duration_boost %d\n",
                i++, (int)p->location.x, (int)p->location.y, p->room_id, p->effects, p->duration_boost);
    }

    fclose(f);
}

static void finalize_recording(long time_stamp)
{
    // This needs to be added so that the game does not end abruptly
    // if there are no key events near the recording end
    game_playback_add_key_event(time_stamp, 0);
    game_playback_next();
    game_playback_add_end_event();
    record_file_flush();
    if (get_game_settings()->require_authentication)
    {
        char hash[DMAC_SHA1_HASH_SIZE];
        dmac_sha1_set_ctx(AUTH_CTX_GAMEPLAY_RECORDING);
        dmac_sha1_calculate_hash_f(hash, game_playback_get_filename());
        char fname[256 + 10];
        sprintf(fname, "%s.auth", game_playback_get_filename());
        FILE *hash_file = fopen(fname, "wb");
        fwrite(hash, 1, DMAC_SHA1_HASH_SIZE, hash_file);
        fclose(hash_file);
    }
}

static void reset_screen_transform()
{
    ALLEGRO_TRANSFORM transform;
    al_identity_transform(&transform);
    al_use_transform(&transform);
}

void game(GlobalGameState *ggs)
{
    pr_reset_random();

    int no_player_damage = ggs->cheats & 1;

    World world;
    memset(&world, 0, sizeof(World));
    ggs->player = &world.plr;
    world.game_modifiers = &ggs->game_modifiers;

    int vibrations = 0;

    read_game_tuning_params();
    init_world(&world);
    read_enemy_configs(&world);
    int mission_count = read_mission_count(ggs->game_modifiers);
    init_player(&world, &ggs->plrautosave);

    long key_press_mask = 0;

    int *record_mode = get_playback_mode();
    long playback_next_event_time_stamp = init_playback(&world, ggs, *record_mode);
    long time_stamp = 0;

    read_level(&world, ggs->mission, 1);
    ggs->custom_next_mission_set = 0;
    if (ggs->next_mission == 0)
        ggs->next_mission = ggs->mission + 1;

    struct fly_in_text fly_in_text;
    set_fly_in_text(&fly_in_text, world.mission_display_name);

    if (world.boss_fight && world.play_boss_sound)
    {
        trigger_sample_with_params(SAMPLE_BOSSTALK_1, 255, 127, 1000);
    }

    set_player_start_state(&world, ggs);

    int plr_dir_helper_intensity = 0;

    create_sparkles(world.plr.x, world.plr.y, 30, -1, 10, &world);

    double game_loop_clk = al_get_time();

    int next_perk_xp = calculate_next_perk_xp(world.plr.perks);

    int old_perks = world.plr.perks;
    if (ggs->setup_screenshot_buffer)
        screenshot(SCREENSHOT_ACT_INIT);
    while (1)
    {
        if (world.plr.health <= 0)
        {
            // Draw well outside of screen so that the zoom in transformation would not look like ass
            al_draw_filled_rectangle(0, 0, SCREEN_W * 2, SCREEN_H * 2, BLACK);
        }
        else if (check_key(get_game_settings()->keys.map_info))
        {
            show_ingame_info_screen(&world);
        }

        cleanup_bodyparts(&world, &world.visual_fx);
        if (time_stamp % 6 == 0)
        {
            reset_sample_triggers();
        }
        time_stamp++;
        draw_map_floors(&world, vibrations);
        move_and_draw_body_parts(&world);
        draw_wall_shadows(&world);
        progress_and_draw_flame_fx(&world.visual_fx);

        // Draw legend to same position as player
        // Legend cannot be drawn here or it would get obscured
        // by walls etc.
        int legend_x = world.plr.x;
        int legend_y = world.plr.y;
        int key_left, key_right, key_up, key_down, key_space,
            key_x, key_z, key_a, key_s, key_d, key_f;
        if (world.plr.health > 0)
        {
            draw_enemy(&world.plr, &world);
            check_perks_changed(&world, old_perks, &next_perk_xp);

            if (*record_mode == RECORD_MODE_PLAYBACK)
            {
                int has_more = 1;
                if (time_stamp >= playback_next_event_time_stamp)
                {
                    key_press_mask = game_playback_get_key_mask();
                    game_playback_next();
                    playback_next_event_time_stamp = game_playback_get_time_stamp();
                    has_more = playback_next_event_time_stamp != -1;
                    LOG("Timestamp=%ld, keymask=0x%lx, Next timestamp: %ld\n",
                        time_stamp, key_press_mask, playback_next_event_time_stamp);
                }
                if (!has_more)
                {
                    world.visual_fx.hint.time_shows = 0;
                    trigger_sample_with_params(SAMPLE_WARP, 255, 127, 500);

                    display_level_info(&world, ggs->mission, ggs->next_mission, mission_count, time_stamp - 1);

                    if (!ggs->no_player_interaction)
                        wait_key_press(ALLEGRO_KEY_ENTER);
                    break;
                }

                key_left = key_press_mask & 1;
                key_right = key_press_mask & 2;
                key_up = key_press_mask & 4;
                key_down = key_press_mask & 8;
                key_space = key_press_mask & 16;
                key_x = key_press_mask & 32;
                key_z = key_press_mask & 64;
                key_a = key_press_mask & 128;
                key_s = key_press_mask & 256;
                key_d = key_press_mask & 512;
                key_f = key_press_mask & 1024;
                if (key_press_mask & 2048)
                    world.plr.perks = (int)((key_press_mask >> 16) & 0xFFFF);
            }
            else
            {
                struct game_control_keys *keys = &get_game_settings()->keys;
                key_left = check_key(keys->left);
                key_right = check_key(keys->right);
                key_up = check_key(keys->up);
                key_down = check_key(keys->down);
                key_space = check_key(keys->shoot);
                key_x = check_key(keys->weapon1);
                key_z = check_key(keys->weapon0);
                key_a = check_key(keys->pwup0);
                key_s = check_key(keys->pwup1);
                key_d = check_key(keys->pwup2);
                key_f = check_key(keys->pwup3);
            }

            world.potion_turbo_mode = (ggs->game_modifiers & GAMEMODIFIER_POTION_ON_DEATH) && key_space && world.potion_duration > 0;

            int player_did_change_dir = handle_direction_keys(&world, key_up, key_down, key_left, key_right);
            if (player_did_change_dir)
            {
                plr_dir_helper_intensity = PLR_DIR_HELPER_INITIAL_INTENSITY;
            }

            handle_shoot_key(&world, key_space);

            handle_weapon_change_keys(&world, key_x, key_z);

            int gold_hint_amount = handle_power_up_keys(&world, key_a, key_s, key_d, key_f);
            if (gold_hint_amount)
            {
                show_gold_hint(&world, &world.visual_fx, gold_hint_amount);
            }

            if (check_key(get_game_settings()->keys.restart) && *record_mode != RECORD_MODE_PLAYBACK && ggs->mission != LIMBO_MISSION)
            {
                wait_key_release(get_game_settings()->keys.restart);
                break;
            }

            if (*record_mode == RECORD_MODE_RECORD)
            {
                long new_key_press_mask = (key_left ? 1 : 0) | (key_right ? 2 : 0) |
                                          (key_up ? 4 : 0) | (key_down ? 8 : 0) |
                                          (key_space ? 16 : 0) | (key_x ? 32 : 0) |
                                          (key_z ? 64 : 0) | (key_a ? 128 : 0) |
                                          (key_s ? 256 : 0) | (key_d ? 512 : 0) |
                                          (key_f ? 1024 : 0);
                if (world.plr.perks != old_perks)
                    new_key_press_mask = new_key_press_mask | 2048 | (world.plr.perks << 16);
                if (new_key_press_mask != key_press_mask)
                {
                    key_press_mask = new_key_press_mask;

                    game_playback_add_key_event(time_stamp, key_press_mask);
                    game_playback_next();
                }
            }
        }
        else
        {
            world.plr.move = 0;
        }
        old_perks = world.plr.perks;
        move_and_reload_plr(&world);

        change_room_if_at_exit_point(&world);

        if (get_tile_at(&world, world.plr.x, world.plr.y)->is_exit_level && world.plr.health > 0)
        {
            world.visual_fx.hint.time_shows = 0;
            trigger_sample_with_params(SAMPLE_WARP, 255, 127, 500);

            display_level_info(&world, ggs->mission, ggs->next_mission, mission_count, time_stamp - 1);

            if (*record_mode != RECORD_MODE_PLAYBACK)
                wait_key_press(ALLEGRO_KEY_ENTER);

            if (ggs->mission == mission_count)
            {
                ggs->mission = -1;
                set_game_mode_beaten_flag(*world.game_modifiers);
                break;
            }

            ggs->mission = ggs->next_mission;
            ggs->plrautosave = world.plr;
            break;
        }

        apply_timed_potion_effects(&world);

        enemy_logic(&world);

        if (world.plr.health > 0)
        {
            int prev_xp = world.plr.xp;
            bullet_logic(&world, ggs);
            if (prev_xp < next_perk_xp && world.plr.xp >= next_perk_xp &&
                (ggs->game_modifiers & GAMEMODIFIER_ARENA_FIGHT) == 0)
            {
                set_fly_in_text(&fly_in_text, "Level up! New perks available!");
            }
            potion_logic(&world);
        }

        draw_map_walls(&world);

        vibrations = calc_vibrations(progress_and_draw_explosions(&world, &world.visual_fx));

        draw_player_legend(&world, legend_x, legend_y);

        draw_hint(&world.visual_fx);
        if (world.boss_fight && time_stamp % 3 == 0)
        {
            boss_logic(&world, 0);
        }

        draw_boss_health_bar(&world);

        display_plr_dir_helper(&world, &plr_dir_helper_intensity);

        draw_rune_of_protection_indicator(&world, &world.visual_fx);

        progress_and_draw_sparkles(&world, &world.visual_fx);

        draw_uber_wizard_weapon_fx(&world.visual_fx);

        draw_fly_in_text(&fly_in_text);

        if ((ggs->game_modifiers & GAMEMODIFIER_ARENA_FIGHT) == 0)
            al_draw_textf(get_font_tiny(), WHITE, 5, SCREEN_H - 10, ALLEGRO_ALIGN_LEFT, "XP: %d / %d", world.plr.xp, next_perk_xp);
        else
            al_draw_textf(get_font_tiny(), WHITE, 5, SCREEN_H - 10, ALLEGRO_ALIGN_LEFT, "Kills: %d", world.kills);

        if (world.plr.health > 0)
        {
            apply_game_screen_transform(vibrations);
            if (time_stamp > 1) // Removes the glitch when starting the game; most visible when restarting the level
                al_flip_display();
        }
        else
        {
            progress_player_death_animation(&world);
            al_flip_display();
            wait_delay_ms(40);
            if (world.plr.reload <= 0)
            {
                reset_screen_transform();
                if (ggs->game_modifiers & GAMEMODIFIER_ARENA_FIGHT)
                {
                    display_arena_fight_end_screen(&world, ggs, record_mode, no_player_damage);
                }
                break;
            }
        }

        game_loop_rest(&game_loop_clk);
        consume_event_queue();

        if (ggs->setup_screenshot_buffer)
        {
            screenshot(SCREENSHOT_ACT_CAPTURE);
            if (check_key(ALLEGRO_KEY_INSERT))
            {
                wait_key_release(ALLEGRO_KEY_INSERT);
                screenshot(SCREENSHOT_ACT_DUMP_TO_DISK);
            }
        }

        if (check_key(ALLEGRO_KEY_ESCAPE))
        {
            if (*record_mode == RECORD_MODE_PLAYBACK)
            {
                break;
            }
            world.visual_fx.hint.time_shows = 0;
            int switch_level = menu(1, ggs);
            if (switch_level)
            {
                break;
            }
            // Perks may have been updated
            world.plr_max_health = (world.plr.perks & PERK_INCREASE_MAX_HEALTH) ?
                get_tuning_params()->max_health_with_perk : get_tuning_params()->max_health;
        }
    }

    if (*record_mode == RECORD_MODE_RECORD)
        finalize_recording(time_stamp);

    if (ggs->setup_screenshot_buffer)
        screenshot(SCREENSHOT_ACT_DESTROY);
    if (*record_mode != RECORD_MODE_NONE)
        write_recording_complete_state_file(&world, ggs, time_stamp);

    al_destroy_bitmap(world.spr);
    add_managed_list(NULL, &world.management_list);

    reset_screen_transform();

    if (!ggs->custom_next_mission_set)
        ggs->next_mission = 0; // 0 = automatically set to mission + 1
}