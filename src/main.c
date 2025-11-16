#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "game.h"
#include "logging.h"
#include "du_constants.h"
#include "world.h"
#include "menu.h"
#include "settings.h"
#include "sample_register.h"
#include "loadindicator.h"
#include "sprites.h"
#include "record_file.h"
#include "game_playback.h"
#include "gen_version_info.h"
#include "midi_playback.h"
#include "synth/wt_sample_loader.h"
#include "sha1/du_dmac.h"
#include "command_line_args.h"

#ifdef ENABLE_LOGGING
int logging_enabled = 0;
#endif

static void print_help();
static void exit_actions();
static void register_samples();
static void main_menu_loop(GlobalGameState *ggs, int *record_mode);

int main(int argc, char **argv)
{
    printf("Destination Underworld " DU_VERSION "\n");

    if (read_cmd_line_arg_int(ARG_HELP, argv, argc))
    {
        print_help();
        return 0;
    }
    else
    {
        printf("Run with --help=1 to display all command line options\n");
    }
    char read_arg[256] = "";
#ifdef ENABLE_LOGGING
    logging_enabled = read_cmd_line_arg_int(ARG_LOGGING, argv, argc);
#endif
    read_cmd_line_arg_str(ARG_PLAYER_DAMAGE, argv, argc, read_arg);
    int player_damage_off = 0;
    if (!strcmp(read_arg, "off"))
    {
        player_damage_off = 1;
        LOG("Player damage off\n");
    }
    read_arg[0] = 0;

    read_cmd_line_arg_str(ARG_REC_MODE, argv, argc, read_arg);
    int *record_mode = get_playback_mode();
    int record_playback_no_user_interaction = 0;
    if (!strcmp(read_arg, "record"))
    {
        *record_mode = RECORD_MODE_RECORD;
        LOG("Record mode active.\n");
    }
    else if (!strcmp(read_arg, "play"))
    {
        *record_mode = RECORD_MODE_PLAYBACK;
        char fname[256];
        FILE *record_input_file = NULL;
        if (read_cmd_line_arg_str(ARG_REC_FILE_MODE, argv, argc, fname))
        {
            record_input_file = fopen(fname, "r");
            game_playback_set_filename(fname);
        }
        if (record_input_file == NULL)
        {
            LOG_FATAL("Valid input file required (--file=<filename>)!!\n");
            return 0;
        }
        fclose(record_input_file);
        record_playback_no_user_interaction = read_cmd_line_arg_int(ARG_REC_NO_KEYPRESS, argv, argc);
        LOG("Playback mode active.\n");
    }
    else if (read_arg[0])
    {
        LOG_FATAL("Record mode must be either 'play' or 'record'.\n");
        return 1;
    }

    read_settings(argv, argc, NULL);

    if (get_game_settings()->require_authentication && *record_mode == RECORD_MODE_PLAYBACK)
    {
        char auth_hash_file[256];
        if (read_cmd_line_arg_str(ARG_REC_AUTH, argv, argc, auth_hash_file))
        {
            FILE *f = fopen(auth_hash_file, "rb");
            char hash[DMAC_SHA1_HASH_SIZE];
            fread(hash, 1, DMAC_SHA1_HASH_SIZE, f);
            fclose(f);
            dmac_sha1_set_ctx(AUTH_CTX_GAMEPLAY_RECORDING);
            int res = dmac_sha1_verify_file(game_playback_get_filename(), hash);
            if (res != 0)
            {
                LOG_FATAL("Authentication failed\n");
                return 1;
            }
        }
        else
        {
            LOG_FATAL("Authentication required: please provide a valid auth-hash-file argument.\n");
            return 1;
        }
    }

    int al_init_res = init_allegro();
    if (al_init_res == 1)
    {
        LOG_FATAL("Initializing Allegro failed\n");
        return 1;
    }

    wt_sample_read_all(DATADIR);
    progress_load_state("Loading game...", 1);
    srand((int)time(NULL));
    int game_modifiers = read_cmd_line_arg_int(ARG_DEF_GMODE, argv, argc);

    progress_load_state("Loading samples...", 1);
    register_samples();

    progress_load_state("Loading sprites...", 1);
    {
        char path[256];
        get_data_filename(path, "sprites.dat");
        read_sprites_from_file(path);
    }

    progress_load_state("Loading menu...", 1);
    randomize_midi_playlist();
    consume_event_queue();
    next_midi_track(-1);
    atexit(exit_actions);

    while (1)
    {
        GlobalGameState ggs;
        memset(&ggs, 0, sizeof(ggs));
        if (player_damage_off)
            ggs.cheats |= 1;
        ggs.game_modifiers = game_modifiers;
        ggs.mission = 1;
        ggs.no_player_interaction = record_playback_no_user_interaction;
        ggs.setup_screenshot_buffer = read_cmd_line_arg_int(ARG_SCREENSHOT, argv, argc);
        main_menu_loop(&ggs, record_mode);
        if (*ggs.mission_pack)
        {
            record_file_flush();
            destroy_registered_samples();
            reset_sprites();

            read_settings(argv, argc, ggs.mission_pack);

            register_samples();
            {
                char path[256];
                get_data_filename(path, "sprites.dat");
                read_sprites_from_file(path);
            }
        }
        else
        {
            break;
        }
    }
    return 0;
}

static void main_menu_loop(GlobalGameState *ggs, int *record_mode)
{
    while (ggs->mission != 0)
    {
        if (*record_mode != RECORD_MODE_PLAYBACK)
            menu(0, ggs);
        else if (!ggs->no_player_interaction)
        {
            al_clear_to_color(al_map_rgb(0, 0, 0));
            al_draw_textf(get_menu_title_font(), al_map_rgb(255, 255, 255), SCREEN_W / 2, SCREEN_H / 2, ALLEGRO_ALIGN_CENTRE, "Press enter to start demo playback");
            al_flip_display();
            wait_key_press(ALLEGRO_KEY_ENTER);
        }
        if (*ggs->mission_pack)
            break;
        while (ggs->mission > 0)
        {
            game(ggs);
            if (*record_mode == RECORD_MODE_PLAYBACK)
            {
                ggs->mission = 0;
                break;
            }
        }
    }
}

static void register_samples()
{
    destroy_registered_samples();
    register_sample(SAMPLE_WARP, "warp", SAMPLE_PRIORITY(HIGH, 1), 0);
    register_sample(SAMPLE_BOSSTALK_1, "boss_level_start", SAMPLE_PRIORITY(HIGH, 2), 0);
    register_sample(SAMPLE_BOSSTALK_2, "boss_level_boss_dies", SAMPLE_PRIORITY(HIGH, 2), 0);
    register_sample(SAMPLE_THROW, "throw", SAMPLE_PRIORITY(LOW, 0), 0);
    register_sample(SAMPLE_SELECT_WEAPON, "select_weapon", SAMPLE_PRIORITY(HIGH, 1), 0);
    register_sample(SAMPLE_HEAL, "powerup_healing", SAMPLE_PRIORITY(HIGH, 1), 0);
    register_sample(SAMPLE_PROTECTION, "powerup_protection", SAMPLE_PRIORITY(HIGH, 1), 0);
    register_sample(SAMPLE_TURRET, "powerup_turret", SAMPLE_PRIORITY(HIGH, 1), 0);
    register_sample(SAMPLE_BLAST, "powerup_megablast", SAMPLE_PRIORITY(HIGH, 1), 0);
    register_sample(SAMPLE_SPAWN, "spawn", SAMPLE_PRIORITY(HIGH, 0), 0);
    register_sample(SAMPLE_POTION(POTION_ID_SHIELD), "potion_shield", SAMPLE_PRIORITY(HIGH, 1), 0);
    register_sample(SAMPLE_POTION(POTION_ID_MINOR_SHIELD), "potion_minor_shield", SAMPLE_PRIORITY(HIGH, 1), 0);
    register_sample(SAMPLE_POTION(POTION_ID_STOP), "potion_stop", SAMPLE_PRIORITY(HIGH, 1), 0);
    register_sample(SAMPLE_POTION(POTION_ID_FAST), "potion_fast", SAMPLE_PRIORITY(HIGH, 1), 0);
    register_sample(SAMPLE_POTION(POTION_ID_BOOST), "potion_boost", SAMPLE_PRIORITY(HIGH, 1), 0);
    register_sample(SAMPLE_POTION(POTION_ID_HEAL), "potion_heal", SAMPLE_PRIORITY(HIGH, 1), 0);
    register_sample(SAMPLE_POTION(POTION_ID_INSTANT_HEAL), "potion_instant_heal", SAMPLE_PRIORITY(HIGH, 1), 0);

    for (int i = 0; i < 6; i++)
    {
        char loadsamplename[100];
        sprintf(loadsamplename, "explosion_%d", i + 1);
        register_sample(SAMPLE_EXPLOSION(i), loadsamplename, SAMPLE_PRIORITY(NORMAL, 0), 1);
        sprintf(loadsamplename, "death_%d", i + 1);
        register_sample(SAMPLE_DEATH(i), loadsamplename, SAMPLE_PRIORITY(NORMAL, 0), 2);
    }

    for (int i = 0; i < 3; i++)
    {
        char loadsamplename[100];
        sprintf(loadsamplename, "blood_splash_%d", i + 1);
        register_sample(SAMPLE_SPLASH(i), loadsamplename, SAMPLE_PRIORITY(NORMAL, 0), 3);
    }

    register_sample(SAMPLE_MENU_CHANGE, "menu_change", SAMPLE_PRIORITY(HIGH, 1), 0);
    register_sample(SAMPLE_MENU_SELECT, "menu_select", SAMPLE_PRIORITY(HIGH, 1), 0);
}

static void exit_actions()
{
    extern int exit_due_to_fatal_error;
    printf("Exiting game...\n");
    if (exit_due_to_fatal_error)
    {
        progress_load_state("FATAL ERROR!", 0);
        wait_delay_ms(3000);
    }
    progress_load_state("Exiting game...", 0);
    destroy_registered_samples();

    wt_sample_free();
    record_file_flush();
    destroy_allegro();
}

static void print_help_for_arg(const char *arg_and_doc)
{
    printf("--%s=<value>\n", arg_and_doc);
    const char *documentation = &arg_and_doc[strlen(arg_and_doc) + 1];
    printf("  %s\n", documentation);
}

static void print_help_for_setting(const char *segment, const char *key_and_doc)
{
    printf("--%s--%s=<value>\n", segment, key_and_doc);
    const char *documentation = &key_and_doc[strlen(key_and_doc) + 1];
    printf("  %s\n", documentation);
}

static void print_help()
{
    puts("Accepted command line arguments:");
    puts("\n** General:");
    print_help_for_arg(ARG_HELP);
    print_help_for_arg(ARG_SCREENSHOT);
    puts("\n** Gameplay recording and playback:");
    print_help_for_arg(ARG_REC_MODE);
    print_help_for_arg(ARG_REC_FILE_MODE);
    print_help_for_arg(ARG_REC_NO_KEYPRESS);
    print_help_for_arg(ARG_REC_AUTH);

    puts("\n** Fine tuning for custom mission packs:");
    print_help_for_arg(ARG_DEF_GMODE);
    print_help_for_arg(ARG_SETTINGS_DAT);

    puts("\n** Debugging:\n");
#ifdef ENABLE_LOGGING
    print_help_for_arg(ARG_LOGGING);
#endif
    print_help_for_arg(ARG_PLAYER_DAMAGE);

    puts("\n** Override game settings:");
    print_help_for_setting(SETTING_MISSION_PACK);
    print_help_for_setting(SETTING_CUSTOM_RES);
    print_help_for_setting(SETTING_REQ_AUTH);
    puts("");
    print_help_for_setting(SETTING_VIBRATE);
    print_help_for_setting(SETTING_FULLSCREEN);
    print_help_for_setting(SETTING_MENU_FONT);
    print_help_for_setting(SETTING_GAME_FONT);
    puts("");
    print_help_for_setting(SETTING_MUSIC_ON);
    print_help_for_setting(SETTING_MUSIC_VOL);
    print_help_for_setting(SETTING_SFX_VOL);
}