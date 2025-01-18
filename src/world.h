#pragma once

#include "allegro42_compat.h"
#include "duconstants.h"
#include "bossfightconf.h"
#include "helpers.h"
#include <time.h>

struct gamedata
{
    char game_id[256];
    long kills;
    long deaths;
    long fireballs;
    long powerups;
};

typedef struct
{
    int exists;
    double velocity;
    int anim;
    int type;
    double x;
    double y;
    double dx;
    double dy;
} BodyPart;

enum TurretType
{
    TURRET_TYPE_NONE,
    TURRET_TYPE_ENEMY,
    TURRET_TYPE_PLAYER
};

typedef struct
{
    int x;
    int y;
    int dx;
    int dy;
    int health;
    int shots;
    int reload;
    int wait;
    int rate;
    int move;
    int anim;
    int ammo;
    int gold;
    int id;
    int former_id;
    int roomid;
    long completetime;
    int fast;
    enum TurretType turret;
    int hurts_monsters;
    int sprite;
    int potion; // if >= 0, the enemy drops a potion with this effect after death
    BodyPart bodyparts[BODYPARTCOUNT];
    int death_animation;
} Enemy;

#define BULLET_TYPE_NORMAL 0
#define BULLET_TYPE_CLUSTER 1

#define BULLET_HURTS_MONSTERS 1
#define BULLET_HURTS_PLAYER 2

typedef struct
{
    double x;
    double y;
    double dx;
    double dy;
    int owner_id;
    int hurts_flags;
    int bullet_type;
    unsigned duration; // How many frames the bullet has been alive?
} Bullet;

#define TILE_SYM_FLOOR 46
#define TILE_SYM_WALL1 120
#define TILE_SYM_WALL2 113
#define TILE_SYM_BREAKABLE_WALL 100
#define TILE_SYM_LAVA 122
#define TILE_SYM_EXIT_LEVEL 60
#define TILE_SYM_EXIT_POINT(x) (1000 + (x))

#define WALL_NORMAL 1
#define WALL_PENTAGRAM 2
#define WALL_LAVA 3

#define GAMEMODIFIER_DOUBLED_SHOTS 0x1
#define GAMEMODIFIER_OVERPOWERED_POWERUPS 0x2
#define GAMEMODIFIER_OVERPRICED_POWERUPS 0x4
#define GAMEMODIFIERS_OVER_POWERUP (GAMEMODIFIER_OVERPOWERED_POWERUPS | GAMEMODIFIER_OVERPRICED_POWERUPS)
#define GAMEMODIFIER_MULTIPLIED_GOLD 0x8
#define GAMEMODIFIER_BRUTAL 0x10
#define GAMEMODIFIER_ARENA_FIGHT 0x20
#define GAMEMODIFIER_POTION_ON_DEATH 0x40
#define GAMEMODIFIER_NO_GOLD 0x80

#define GET_DIFFICULTY(world) (((world)->game_modifiers & GAMEMODIFIER_BRUTAL) != 0 ? DIFFICULTY_BRUTAL : DIFFICULTY_NORMAL)

typedef struct
{
    int data;
    // Flags
    int is_exit_level;
    int is_floor;
    int is_blocker;
    int is_restricted;
    int is_clear_restriction;
    int is_positional_trigger;
    int is_exit_point;
    int is_wall;
    int is_blood_stained;
    int durability;
    int valid;
} Tile;

Tile create_tile(int symbol);

struct explosion_circle
{
    // Relative position
    Coordinates loc;
    // Intensity
    double i;
    // Radius
    double r;
};

typedef struct
{
    int exists;
    int x;
    int y;
    int phase;
    int circle_count;
    double intensity;
    struct explosion_circle circles[10];
} Explosion;

struct hint_text
{
    Coordinates loc;
    int dim;
    int time_shows;
    char text[256];
};

struct powerup_status
{
    int cluster_strength;
    int rune_of_protection_active;
};

struct sparkle_fx
{
    Coordinates loc;
    Coordinates dir;
    int sprite;
    int duration;
    int color;
};

struct sparkle_fx_circle
{
    Coordinates loc;
    int duration;
    int time;
    ALLEGRO_COLOR color;
};

struct enemy_config
{
    int turret;
    int rate;
    int health;
    int gold;
    int fast;
    int hurts_monsters;
    int potion_for_potion_only;
};

#define POTION_EFFECT_SHIELD_OF_FIRE (1 << 0)
#define POTION_EFFECT_STOP_ENEMIES (1 << 1)
#define POTION_EFFECT_FAST_PLAYER (1 << 2)
#define POTION_EFFECT_BOOSTED_SHOTS (1 << 3)
#define POTION_EFFECT_ALL_BULLETS_HURT_MONSTERS (1 << 4)
#define POTION_EFFECT_HEALING (1 << 5)
#define POTION_EFFECT_HEAL_ONCE (1 << 6)
#define POTION_DURATION_CAP 400

typedef struct potion
{
    Coordinates location;
    int room_id;
    int duration_boost;
    int effects;
    int exists;
    int sprite;
    int sample;
} Potion;

typedef struct
{
    Tile map[ROOMCOUNT][MAPMAX_X][MAPMAX_Y];
    char floor_shade_map[ROOMCOUNT][MAPMAX_X][MAPMAX_Y];
    char rooms_visited[ROOMCOUNT];
    int current_room;
    Enemy enm[ENEMYCOUNT];
    Enemy plr;
    Enemy *boss;
    Bullet bullets[BULLETCOUNT];
    Explosion explosion[EXPLOSIONCOUNT];
    struct sparkle_fx sparkle_fx[SPARKLE_FX_COUNT];
    struct sparkle_fx_circle sparkle_fx_circle[SPARKLE_FX_CIRCLE_COUNT];
    struct enemy_config enemy_configs[ENEMY_TYPE_COUNT];

    ALLEGRO_BITMAP *spr;

    int boss_fight;
    int play_boss_sound;
    int game_modifiers;
    BossFightConfig *boss_fight_config;
    BossFightConfig boss_fight_configs[ROOMCOUNT];
    struct hint_text hint;
    struct powerup_status powerups;
    int playcount;
    char mission_display_name[64];
    int story_after_mission_lines;
    // ~60 characters fit on the screen
    char story_after_mission[10][61];
    double par_time;
    int kills;
    int mission;
    int final_level;
    float map_wall_color[3];

    int potion_duration;
    int potion_effect_flags;
    int potion_turbo_mode;
    int potion_healing_counter;
    int potion_shield_counter;
    Potion potions[POTION_COUNT];
} World;

int check_potion_effect(World *w, int effect_id);

void clear_visual_fx(World *);
void stop_bodyparts(World *);
void init_world(World *world);
void spawn_body_parts(Enemy *enm);

Tile *get_tile_at(World *world, int x, int y);
int get_wall_type_at(World *world, int x, int y);
// no scale versions
Tile *ns_get_tile_at(World *world, int x, int y);
int ns_get_wall_type_at(World *world, int x, int y);
void init_player(World *world, Enemy *plrautosave);
void cleanup_bodyparts(World *world);
