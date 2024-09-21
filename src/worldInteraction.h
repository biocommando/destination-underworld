#pragma once

#include "world.h"
#include "arenaconf.h"

void move_enemy(Enemy *enm, World *world);
void create_shade_around_hit_point(int x, int y, int spread, World *world);
void move_bullet(Bullet *bb, World *world);
Bullet *get_next_available_bullet(World *world);
int shoot(Enemy *enm, World *world);
int shoot_one_shot_at_xy(double x, double y, double dx, double dy, Enemy *enm, int hurts_monsters, World *world);
int bullet_hit(Enemy *enm, Bullet *bb);
int sees_each_other(Enemy *e1, Enemy *e2, World *world);
void create_explosion(int x, int y, World *world, double intensity);
void create_sparkles(int x, int y, int count, int color, World *world);
int read_level(World *world, int mission, int room_to);
Enemy *get_next_available_enemy(World *world, int *index);
Enemy *spawn_enemy(int x, int y, int type, int room_id, World *world);
Enemy *ns_spawn_enemy(int x, int y, int type, int room_id, World *world);
void create_cluster_explosion(World *w, double x0, double y0, int num_directions, int intensity, Enemy *enm);
void change_room_if_at_exit_point(World *world, int mission);
void read_enemy_configs(World *world);
int parse_highscore_from_world_state(World *world, ArenaHighscore *highscore, int *hs_arena, int *hs_mode);
Potion *spawn_potion(int x, int y, int type, int room_id, World *world,
    int range_start, int range_end);
