#ifndef WORLD_H
#define WORLD_H
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
    BodyPart bodyparts[BODYPARTCOUNT];
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
} Bullet;

#define TILE_IS_EXIT_LEVEL /*-------*/ 0x01
#define TILE_IS_FLOOR /*------------*/ 0x02
#define TILE_IS_BLOCKER /*----------*/ 0x04
#define TILE_IS_RESTRICTED /*-------*/ 0x08
#define TILE_IS_CLEAR_RESTRICTION /**/ 0x10
#define TILE_IS_EXIT_POINT /*-------*/ 0x20
#define TILE_UNRECOGNIZED /*--------*/ 0x40
#define TILE_IS_WALL /*-------------*/ 0x80
#define TILE_IS_BLOOD_STAINED /*----*/ 0x100

#define TILE_SYM_FLOOR 46
#define TILE_SYM_WALL1 120
#define TILE_SYM_WALL2 113
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

#define GET_DIFFICULTY(world) (((world)->game_modifiers & GAMEMODIFIER_BRUTAL) != 0 ? DIFFICULTY_BRUTAL : DIFFICULTY_NORMAL)

typedef struct
{
    int flags;
    int data;
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
};

struct enemy_config
{
    int turret;
    int rate;
    int health;
    int gold;
    int fast;
    int hurts_monsters;
};

#define POTION_EFFECT_SHIELD_OF_FIRE 1
#define POTION_EFFECT_STOP_ENEMIES 2
#define POTION_EFFECT_FAST_PLAYER 4
#define POTION_EFFECT_BOOSTED_SHOTS 8
#define POTION_EFFECT_ALL_BULLETS_HURT_MONSTERS 16
#define POTION_EFFECT_HEALING 32
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
    Tile map[MAPMAX_X][MAPMAX_Y];
    char floor_shade_map[ROOMCOUNT][MAPMAX_X][MAPMAX_Y];
    char rooms_visited[ROOMCOUNT];
    int current_room;
    Enemy enm[ENEMYCOUNT];
    Enemy plr;
    Enemy *boss;
    Bullet bullets[BULLETCOUNT];
    Explosion explosion[EXPLOSIONCOUNT];
    struct sparkle_fx sparkle_fx[SPARKLE_FX_COUNT];
    struct enemy_config enemy_configs[5];

    BITMAP *spr;

    int boss_fight;
    int play_boss_sound;
    int game_modifiers;
    BossFightConfig boss_fight_config;
    Coordinates boss_waypoint;
    int boss_want_to_shoot;
    struct hint_text hint;
    struct powerup_status powerups;
    int playcount;
    char mission_display_name[64];
    int story_after_mission_lines;
    // ~60 characters fit on the screen
    char story_after_mission[10][61];
    // Flag telling if the level has been read yet.
    // (Level is read whenever room is changed but
    // there's no point in parsing the metadata each time.)
    int level_read;
    double par_time;
    int kills;
    int mission;
    int final_level;
    float map_wall_color[3];

    int potion_duration;
    int potion_effect_flags;
    Potion potions[POTION_COUNT];
} World;

int check_potion_effect(World *w, int effect_id);

void clear_visual_fx(World *);
void stop_bodyparts(World *);
void init_world(World *world);
void spawn_body_parts(Enemy *enm);

Tile get_tile_at(World *world, int x, int y);
int check_flags_at(World *world, int x, int y, int flags_to_check);
int get_wall_type_at(World *world, int x, int y);
// no scale versions
Tile ns_get_tile_at(World *world, int x, int y);
int ns_check_flags_at(World *world, int x, int y, int flags_to_check);
int ns_get_wall_type_at(World *world, int x, int y);
void set_tile_flag(World *world, int x, int y, int flags);
void init_player(World *world, Enemy *plrautosave);
void cleanup_bodyparts(World *world);

#endif
