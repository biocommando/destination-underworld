#pragma once

#include "world.h"
#include "arenaconf.h"

/*
 * Moves the enemy one position to the direction indicated by the dx, dy vector if
 * the move flag is set to true.
 * Checks the map that the direction is not blocked, otherwise does not do the move.
 * For player, checks the positional trigger tiles and restriction clear tiles.
 *
 * Also progresses walking animation state
 */
void move_enemy(Enemy *enm, World *world);
/*
 * Progresses reload status. This was in move_enemy and in effect, if the move_enemy
 * function is called multiple times, it also affects the fire rate. As technically this is a
 * bug, let's move the logic outside and make it explicit.
 */
void enemy_reload(Enemy *enm, int amount);
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
 * Gets arena highscores from the GameSettings.arena_config.arenas data structure. If the highscore.mode
 * array does not contain the arena/game mode mapping, the function reserves a slot for it.
 *
 * hs_arena and hs_mode are set to the indices in ArenaHighscore arrays so that the data can be accessed
 * by the caller like:
 * `highscore->kills[*hs_arena][*hs_mode] = 100;`
 */
int parse_highscore_from_world_state(const World *world, int mission, ArenaHighscore *highscore, int *hs_arena, int *hs_mode);
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

/*
 * Modifies the player start state so that it takes game mode, perks and
 * possible scripted property overrides into account. The player's state
 * without calling this would be the same as the end state when exiting a
 * level.
 */
void set_player_start_state(World *world, GlobalGameState *ggs);

/*
 * Apply potion effects for healing and shield of fire where the effects are time based.
 */
void apply_timed_potion_effects(World *world);

/*
 * Create a powerup turret at player's location.
 */
Enemy *create_turret(World *world);

/*
 * Does all the required actions when an enemy dies.
 */
void kill_enemy(Enemy *enm, World *world);