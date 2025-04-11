#pragma once

#include "allegro_management.h"
#include "duconstants.h"
#include "bossfightconf.h"
#include "helpers.h"
#include <time.h>

// Body part that's left behind when an enemy dies
typedef struct
{
    // 1 = show the body part
    int exists;
    // Speed of the movement
    double velocity;
    // Animation frame
    int anim;
    // Sprite index on the spritesheet
    int type;
    // x position on the screen
    double x;
    // y position on the screen
    double y;
    // x movement vector
    double dx;
    // y movement vector
    double dy;
} BodyPart;

enum TurretType
{
    TURRET_TYPE_NONE,
    TURRET_TYPE_ENEMY,
    TURRET_TYPE_PLAYER
};

#define PERK_INCREASE_MAX_HEALTH 1
#define PERK_IMPROVE_HEALTH_POWERUP 2
#define PERK_IMPROVE_TURRET_POWERUP 4
#define PERK_IMPROVE_SHIELD_POWERUP 8
#define PERK_IMPROVE_BLAST_POWERUP 16
#define PERK_START_WITH_SPEED_POTION 32
#define PERK_START_WITH_SHIELD_POWERUP 64

// An enemy, player or player's powerup turret
typedef struct
{
    // x position on the screen
    int x;
    // y position on the screen
    int y;
    // x movement direction (-1, 0 or 1)
    int dx;
    // y movement direction (-1, 0 or 1)
    int dy;
    // Current health
    int health;
    // How many shots the enemy shoots at a time
    int shots;
    // Set to 'rate' when a shot is fired. The enemy can
    // shoot again only when this field reaches zero.
    int reload;
    // Fire rate (frames between shots)
    int rate;
    // Used for two things:
    // - Checking if the enemy/player should move in move_enemy function
    // - Determining the total travel distance and speed for player turret
    int move;
    // Animation frame
    int anim;
    // How many shots the player can shoot still. -1 = infinite ammo.
    int ammo;
    // Player gold amount, or the amount of gold the player gets when killing the enemy
    int gold;
    // Used for checking alive status
    int alive;
    // Used for checking if the enemy has been alive (but now dead). This is used for
    // showing the "corpse".
    int killed;
    // The room in which the enemy is located
    int roomid;
    // Fast enemies move at double speed
    int fast;
    // Turret type for turret type enemy
    enum TurretType turret;
    // True if the bullets the enemy shoots hurts the enemies
    int hurts_monsters;
    // Sprite index
    int sprite;
    // if >= 0, the enemy drops a potion with this effect after death
    int potion;
    // Body parts for this enemy (shown if !alive && killed)
    BodyPart bodyparts[BODYPARTCOUNT];
    // After death, when this counter reaches a certain value, the enemy will spawn the body parts
    int death_animation;
    // Experience points for player and the amount of xp that killing one enemy gives the player
    int xp;
    // Perks can be bought with experience points. Each perk will cost more than the previously bought.
    // This adds a lite RPG element to the game.
    int perks;
} Enemy;

#define BULLET_TYPE_NORMAL 0
#define BULLET_TYPE_CLUSTER 1

#define BULLET_HURTS_MONSTERS 1
#define BULLET_HURTS_PLAYER 2

// A single shot fired by player or enemy
typedef struct
{
    // x position on the screen
    double x;
    // y position on the screen
    double y;
    // x speed vector
    double dx;
    // y speed vector
    double dy;
    // The Enemy that "owns" the bullet. Used for
    // checking if the bullet exists or not.
    // The bullet needs to know its owner so that it doesn't hurt the owner
    // although both hurt flags would be set.
    Enemy *owner;
    // What actor types the bullet can hurt? (see BULLET_HURTS_* definitions)
    int hurts_flags;
    // The bullet type (see BULLET_TYPE_* definitions)
    int bullet_type;
    // How many frames the bullet has been alive?
    unsigned duration;
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
#define GAMEMODIFIER_UBER_WIZARD 0x100

#define GET_DIFFICULTY(world) ((*(world)->game_modifiers & GAMEMODIFIER_BRUTAL) != 0 ? DIFFICULTY_BRUTAL : DIFFICULTY_NORMAL)

// Map tile defining all the information for a single tile on a map
typedef struct
{
    // Has different functions depending on other flags.
    // Can mean:
    // - restriction group for restricted and clear restriction tiles
    // - wall type (affects the drawn sprite) for walls
    // - destination room id for exit points
    // - positional trigger id
    int data;
    // Flags

    // Acts as level exit
    int is_exit_level;
    // Is plain floor (no exit level etc.).
    // Separated from other floor types so that we can determine
    // where to draw wall shadows.
    int is_floor;
    // Blocks movement
    int is_blocker;
    // Means that enemies cannot pass the tile.
    // data field tells the restriction group.
    // Note that restricted tile will actually restrict the movement
    // only if also the is_blocker flag is set.
    int is_restricted;
    // Entering the tile clears is_restricted flag for restricted tiles
    // in the group determined by the data field.
    int is_clear_restriction;
    // Acts as a positional trigger point for a scripted event; activates
    // when the player enters the tile.
    int is_positional_trigger;
    // Stepping on the tile transfers the player to another room
    int is_exit_point;
    // The tile is a wall
    int is_wall;
    // The tile has a blood stain on it (=an enemy has died on it)
    int is_blood_stained;
    // How many shots the wall can stand. At 0 the wall is replaced with a floor.
    int durability;
    // Is read tile info valid? Used when reading levels.
    int valid;
} Tile;

Tile create_tile(int symbol);

// Explosion "circle group" visual effect
struct explosion_circle
{
    // Relative position
    Coordinates loc;
    // Intensity
    double i;
    // Radius
    double r;
};

// Explosion visual effect
typedef struct
{
    // 1 = draw effect
    int exists;
    // x position for the explosion center
    int x;
    // y position for the explosion center
    int y;
    // How long the explosion has lasted. At 31 the explosion won't be drawn anymore.
    // Affects the effect intensity.
    int phase;
    // How many "circle groups" are drawed for the explosion
    int circle_count;
    // Affects the speed of the expanding circle of fire balls drawn in the beginning
    // of the effect.
    double intensity;
    // Each individual "circle group"
    struct explosion_circle circles[10];
    // Does the explosion have a blast wave effect? This is change based.
    int emit_blast_wave;
} Explosion;

/*
 * A text that is shown for a short moment at a fixed location and that fades away.
 * Currently used for showing arena kill counts and increments in gold amount.
 */
struct hint_text
{
    // Position on screen (no centering)
    Coordinates loc;
    // How dark the text is currently. The text gets dimmer all the time before disappearing.
    int dim;
    // When this reaches zero, the text is shown no more
    int time_shows;
    char text[256];
};

// Current status of powerups that don't affect some property directly
struct powerup_status
{
    // How many shots there are in the "cluster shot"
    int cluster_strength;
    // >0 = the "rune of protection" powerup is active.
    // 1 = large blast
    // >1 = smaller blasts
    // <0 = animation when the powerup has been depleted
    int rune_of_protection_active;
};

// A visual effect where a bunch of "sparkles" are spawned at a location
// and the sparkles go to all direction before they disappear.
struct sparkle_fx
{
    // Position on screen
    Coordinates loc;
    // Direction (and speed) where the sparkle is headed to
    Coordinates dir;
    // Sprite index in the spritesheet that selects the animation
    // phase
    int sprite;
    // When this reaches zero the effect is no longer drawn
    int duration;
    // Sprite index in the spritesheet that selects the color
    int color;
};

// An expanding circle spawned at the same location as the sparkle_fx
struct sparkle_fx_circle
{
    // Center position
    Coordinates loc;
    // When this reaches zero the effect is no longer drawn
    int duration;
    // This is incremented each frame. This will determine the
    // radius of the circle
    int time;
    ALLEGRO_COLOR color;
};

// A glowing ember going up in a wavy motion
struct flame_ember_fx
{
    // Circle position
    Coordinates loc;
    // Speed (used for both vertical and horizontal movement)
    double speed;
    // Current color (gets dimmer all the time)
    ALLEGRO_COLOR color;
    // The size of the ember
    int r;
};

// A flame that is left behind when anything explodes
struct flame_fx
{
    // Center position
    Coordinates loc;
    // When this reaches zero the effect is no longer drawn.
    // The number of embers drawn depends on duration so the
    // flame will slowly die out.
    // When the duration reaches 1 the logic waits for the
    // last ember to die out too.
    int duration;
    // The actual visual effect
    struct flame_ember_fx embers[EMBERS_PER_FLAME_FX];
};

// A "template" for an enemy
struct enemy_config
{
    // 1 = enemy is static (can't move)
    int turret;
    // Fire rate (frames between shots)
    int rate;
    // Enemy initial health
    int health;
    // How much player gets gold when they kill the enemy
    int gold;
    // 1 = the enemy moves a bit faster
    int fast;
    // Shots fired hurt also other enemies
    int hurts_monsters;
    // The id for potion that the enemy drops.
    // Only valid in "potion only" game mode
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
#define POTION_DURATION_BIG_BOOST 250
#define POTION_DURATION_MINI_BOOST 50

// A pickable potion
typedef struct potion
{
    // Location on the map (pixel coordinates)
    Coordinates location;
    // The room in which the potion is
    int room_id;
    // How much the potion increases the total potion duration
    int duration_boost;
    // A bit mask for potion effects (see POTION_EFFECT_* definitions)
    int effects;
    // 1 = yes, 0 = no
    int exists;
    // Potion sprite index in spritesheet
    int sprite;
    // Sample index to play when the potion is picked up
    int sample;
} Potion;

// Visual FX for Uber Wizard's weapon
typedef struct 
{
    Coordinates start;
    Coordinates end;
    int dim;
    int type;
} UberWizardWeaponFx;

// Structure that contains most of the game state
typedef struct
{
    // The tiles for each room
    Tile map[ROOMCOUNT][MAPMAX_X][MAPMAX_Y];
    // Floor color map; the floor gets darker always when there's an explosion
    // at that tile
    char floor_shade_map[ROOMCOUNT][MAPMAX_X][MAPMAX_Y];
    // Set to 1 for each room that has been already visited so that
    // enemies and potions won't be spawned again
    char rooms_visited[ROOMCOUNT];
    // Room number currently active
    int current_room;
    // All enemies and other actors (but not the player)
    Enemy enm[ENEMYCOUNT];
    // The player
    Enemy plr;
    // Points to the enemy that is the "boss" or NULL if not present
    Enemy *boss;
    // All shots fired by anything
    Bullet bullets[BULLETCOUNT];
    // Visual effects
    Explosion explosion[EXPLOSIONCOUNT];
    // Visual effects
    struct sparkle_fx sparkle_fx[SPARKLE_FX_COUNT];
    // Visual effects
    struct sparkle_fx_circle sparkle_fx_circle[SPARKLE_FX_CIRCLE_COUNT];
    // Visual effects
    struct flame_fx flames[FLAME_FX_COUNT];
    // Visual effects
    UberWizardWeaponFx uber_wizard_weapon_fx;
    // Enemy type mapping
    struct enemy_config enemy_configs[ENEMY_TYPE_COUNT];

    ALLEGRO_BITMAP *spr;

    // Set to 1 if the room has a script
    int boss_fight;
    // Set to 1 if the boss call sound is wanted 
    int play_boss_sound;
    // Game modifier flags (see GAMEMODIFIER_* definitions). Points to GlobalGameState.
    int *game_modifiers;
    // Points to current room's config
    BossFightConfig *boss_fight_config;
    // Script configs for each room
    BossFightConfig boss_fight_configs[ROOMCOUNT];
    // Status for a text that is shown for a short moment
    struct hint_text hint;
    // The status of power up effects that do not affect directly
    // some other property.
    struct powerup_status powerups;
    // Mission name that is shown in the beginning of the map
    char mission_display_name[64];
    // Number of lines of story to show after the mission
    int story_after_mission_lines;
    // ~60 characters fit on the screen
    char story_after_mission[10][61];
    char custom_story_image[512];
    // Total kills in all rooms in the current map
    int kills;
    // RGB color for map's walls
    float map_wall_color[3];

    // When this reaches zero, all potion effect flags are cleared.
    int potion_duration;
    // All active effects; the potion effects stack on each other so
    // picking up any potion increases the duration for all stacked
    // effects. This is an important game mechanism, especially for the
    // "potion only" mode. (see POTION_EFFECT_* definitions)
    int potion_effect_flags;
    // Makes the potions more effective
    int potion_turbo_mode;
    // Increments player health when this is reduced to zero
    int potion_healing_counter;
    // Shoots fireballs when this is reduced to zero
    int potion_shield_counter;
    Potion potions[POTION_COUNT];
    // Maximum health that can be obtained using potions
    int plr_max_health;
} World;

// Global state that is persisted over different game(...) calls.
typedef struct
{
    // Game mode
    int game_modifiers;
    // Current mission number
    int mission;
    // The initial player status at beginning of a level
    Enemy plrautosave;
    // Bitmask of cheats active.
    // Currently only bit at 0 = no player damage
    int cheats;
    // Don't require keypresses in record playback mode
    int no_player_interaction;
    // Pointer to player for ingame menu (can be null or undefined otherwise)
    Enemy *player;
    // Set to 1 to allow saving a buffer of screenshots (slow) with a keypress
    int setup_screenshot_buffer;
} GlobalGameState;


/*
 * Check if effect with effect_id is active (1 = true, 0 = false).
 */
int check_potion_effect(World *w, int effect_id);
/**
 * Initializes world for starting a new game (e.g. map change).
 */
void init_world(World *world);

/*
 * Get the tile at x, y position (pixel positions).
 */
Tile *get_tile_at(World *world, int x, int y);
/*
 * Get the wall type (see WALL_* macros) at x, y position (pixel positions).
 * Returns 0 if the tile is not a wall.
 */
int get_wall_type_at(World *world, int x, int y);

/*
 * Get the tile at x, y position (tile positions).
 */
Tile *ns_get_tile_at(World *world, int x, int y);
/*
 * Get the wall type (see WALL_* macros) at x, y position (tile positions).
 * Returns 0 if the tile is not a wall.
 */
int ns_get_wall_type_at(World *world, int x, int y);
/*
 * Inits player. Basically copies plrautosave struct if owner is alive.*/
void init_player(World *world, Enemy *plrautosave);

/*
 * Calculates the player speed (affecting movement and fire rate).
 */
int get_plr_speed(World *world);