#pragma once

#include "world.h"
#include "arenaconf.h"

/*
 * Moves the enemy one position to the direction indicated by the dx, dy vector if
 * the move flag is set to true.
 * Checks the map that the direction is not blocked, otherwise does not do the move.
 * For player, checks the positional trigger tiles and restriction clear tiles.
 *
 * Also progresses walking animation state and reload status. In effect, if the move_enemy
 * function is called multiple times, it also affects the fire rate (technically this is a
 * bug but the game has been play tested so much with this feature in action that I'll let
 * it be).
 */
void move_enemy(Enemy *enm, World *world);
/*
 * Creates a circular "crater" around the x, y center position, with the radius of spread,
 * where the closest tiles get the darkest. Modifies the World.floor_shade_map array.
 */
void create_shade_around_hit_point(int x, int y, int spread, World *world);
/*
 * Moves bullet one position to the direction indicated by the dx, dy vector.
 * Checks the map that the direction is not blocked, otherwise destroys the bullet and creates
 * a "crater" on the floor around the hit point. To see if the bullet exists after this function,
 * one needs to check the owner field.
 */
void move_bullet(Bullet *bb, World *world);
/*
 * Gets a bullet from the array that's not in use.
 */
Bullet *get_next_available_bullet(World *world);
/*
 * Make the enemy shoot once. Sets the Enemy.reload field according to fire rate and modifiers and
 * decrements Enemy.ammo by one (if not set to infinite = -1).
 * Shoots as many shots as indicated by Enemy.shots field.
 * Returns 1 if shoot sample should be played.
 */
int shoot(Enemy *enm, World *world);
/*
 * Shoot one shot at coordinates x, y to direction dx, dy. The bullet will be marked to be owned by
 * enm. The fucntion checks all modifiers that should be applied globally to all bullets. Returns 0 if
 * direction vector is 0 vector.
 */
int shoot_one_shot_at_xy(double x, double y, double dx, double dy, Enemy *enm, int hurts_monsters, World *world);
/*
 * Checks if bullet hits an enemy. Does not check if the bullet can actually hurt the enemy, but will just
 * decrement its health by one on hit. On hit destroys the bullet. If the enemy's health reaches zero, will
 * start the death animation and set Enemy.killed/alive flags accordingly.
 * Returns 1 if the bullet hit the enemy, 0 if not.
 */
int bullet_hit(Enemy *enm, Bullet *bb);
/*
 * Checks if two enemies can see each other (there's no wall between their line of sight).
 * Returns 1 if enemies see each other, 0 if not.
 */
int sees_each_other(Enemy *e1, Enemy *e2, World *world);
/*
 * Creates one explosion with the center at position x, y.
 */
void create_explosion(int x, int y, World *world, double intensity);
/*
 * Creates one sparkle effect with a sparkle circle effect at x, y. The count parameter tells the
 * number of sparkles to create. The parameter color is a number from 0 to 2 or -1 for random color
 * for each sparkle. The parameter circle_duration will affect the sparkle circle effect duration only.
 */
void create_sparkles(int x, int y, int count, int color, int circle_duration, World *world);
/*
 * Read level from a file. The file name is either:
 * [GameSettings.mission_pack]\mission[mission]-mode-[World.game_modifiers]
 * or if that's not found:
 * [GameSettings.mission_pack]\mission[mission]
 *
 * The level is read once, and that will set the world state for all rooms.
 * The function has the room_to parameter which is a remnant from old code where
 * levels were read only when entering a new unvisited room.
 *
 * The level file uses duscript format which is a simple scripting format that
 * has a simple support for variables and condition based branching.
 */
int read_level(World *world, int mission, int room_to);
/*
 * Gets an enemy from the array that's not in use.
 * Tries to first use all enemies from the array before reusing enemies that have already
 * died, before resorting to just the first enemy in the array.
 */
Enemy *get_next_available_enemy(World *world);
/*
 * Places a new enemy of a certain type at x, y (map coordinates) in room room_id.
 * Uses ns_spawn_enemy internally.
 */
Enemy *spawn_enemy(int x, int y, int type, int room_id, World *world);
/*
 * Places a new enemy of a certain type at x, y (screen coordinates) in room room_id.
 * Uses the World.enemy_configs for populating the Enemy structure, and checks all the
 * flags affecting enemies globally. If type == ENEMY_TYPE_COUNT, will create a boss enemy.
 * Returns the newly created enemy.
 *
 * Does not create any visual effect automatically.
 */
Enemy *ns_spawn_enemy(int x, int y, int type, int room_id, World *world);
/*
 * Shoots a circle of bullets at coordinate x0, y0. The total count of bullets is num_directions * intensity:
 * num_directions determines how angles are used and intensity tells how many shots to shoot at every direction.
 * The shots are as if fired directly by the enemy enm.
 */
void create_cluster_explosion(World *w, double x0, double y0, int num_directions, int intensity, Enemy *enm);
/*
 * Set the current room to another room if the player is at a room exit point and initialize the next room.
 */
void change_room_if_at_exit_point(World *world);
/*
 * Read enemy configurations from file enemy-properties.dat.
 * Uses the record file format with the following structure:
 * type-%d turret=%d rate=%d health=%d gold=%d fast=%d hurts-monsters=%d  potion-for-potion-only=%d
 *
 * Note that the key values are just for human-readability, the key-value order needs to be exactly like this.
 */
void read_enemy_configs(World *world);
/*
 * Gets arena highscores from the GameSettings.arena_config.arenas data structure. If the highscore.mode
 * array does not contain the arena/game mode mapping, the function reserves a slot for it.
 *
 * hs_arena and hs_mode are set to the indices in ArenaHighscore arrays so that the data can be accessed
 * by the caller like:
 * `highscore->kills[*hs_arena][*hs_mode] = 100;`
 */
int parse_highscore_from_world_state(World *world, ArenaHighscore *highscore, int *hs_arena, int *hs_mode);
/*
 * Spawns a potion of a certain type at x, y (screen coordinates) in room room_id. The first indices in the
 * World.potions array are reserved for the potions that come from the level file and the rest can be used for
 * potion drops. Therefore, the range start and end should be provided. They must be either:
 * POTION_PRESET_RANGE_START, POTION_PRESET_RANGE_END
 * or
 * POTION_DROP_RANGE_START, POTION_DROP_RANGE_END
 *
 * The duration_boost for the potion is determined from the range (drops give a shorter boost).
 */
Potion *spawn_potion(int x, int y, int type, int room_id, World *world,
                     int range_start, int range_end);
