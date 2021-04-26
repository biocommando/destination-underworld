#ifndef WORLD_H
#define WORLD_H
#include "allegro.h"
#include "duconstants.h"
#include "bossfightconf.h"

typedef enum EnemyTypeEnum
{
        ADEPT = 1,
        MAGICIAN = 2,
        IMP = 3,
        ALIEN = 4,
        ALIEN_TURRET = 5,
        ARCH_MAGE = 6,
        TURRET = 7,
        PLAYER = 8,
        NOT_SET = 9
} EnemyType;

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
        int velocity;
        int anim;
        int type;
        double x;
        double y;
        double dx;
        double dy;
} BodyPart;

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
        EnemyType type;
        BodyPart bodyparts[BODYPARTCOUNT];
} Enemy;

#define BULLET_TYPE_NORMAL 0
#define BULLET_TYPE_CLUSTER 1

typedef struct
{
        double x;
        double y;
        double dx;
        double dy;
        int owner_id;
        int hurts_monsters;
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
#define TILE_SYM_EXIT_POINT(x) (1000+(x))

#define WALL_NORMAL 1
#define WALL_PENTAGRAM 2
#define WALL_LAVA 3

#define GAMEMODIFIER_DOUBLED_SHOTS 0x1
#define GAMEMODIFIER_OVERPOWERED_POWERUPS 0x2
#define GAMEMODIFIER_OVERPRICED_POWERUPS 0x4
#define GAMEMODIFIER_MULTIPLIED_GOLD 0x8
#define GAMEMODIFIER_BRUTAL 0x10

typedef struct
{
        int flags;
        int data;
} Tile;

Tile create_tile(int symbol);

typedef struct
{
        int exists;
        int x;
        int y;
        int sprite;
        int phase;
} Explosion;

typedef struct
{
        Tile map[MAPMAX_X][MAPMAX_Y];
        char floor_shade_map[ROOMCOUNT][MAPMAX_X][MAPMAX_Y];
        char rooms_visited[ROOMCOUNT];
        int current_room;
        Enemy enm[ENEMYCOUNT];
        Enemy plr;
        Bullet bullets[BULLETCOUNT];
        Explosion explosion[EXPLOSIONCOUNT];

        BITMAP *buf;
        BITMAP *spr;
        BITMAP *explos_spr;

        int boss_fight;
        int game_modifiers;
        BossFightConfig boss_fight_config;
} World;

void clear_explosions(World *);
void stop_bodyparts(World *);
void init_world(World *world);
void spawn_body_parts(Enemy *enm);

Tile get_tile_at(World *world, int x, int y);
int check_flags_at(World *world, int x, int y, int flags_to_check);
int get_wall_type_at(World *world, int x, int y);
// no scale versions
Tile ns_get_tile_at(World *world, int x, int y);
int ns_check_flags_at(World *world, int x, int y, int flags_to_check);
int ns_getWallTypeAt(World *world, int x, int y);
void set_tile_flag(World *world, int x, int y, int flags);
void init_player(World *world, Enemy *plrautosave);

#endif
