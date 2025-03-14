#include "keyhandling.h"
#include <stdio.h>
#include "duConstants.h"
#include "worldInteraction.h"
#include "sampleRegister.h"

int handle_direction_keys(World *world, int key_up, int key_down, int key_left, int key_right)
{
  int orig_dx = world->plr.dx;
  int orig_dy = world->plr.dy;
  world->plr.move = 0;
  if (key_left)
  {
    world->plr.dx = -1;
    world->plr.dy = 0;
    world->plr.move = 1;
  }
  else if (key_right)
  {
    world->plr.dx = 1;
    world->plr.dy = 0;
    world->plr.move = 1;
  }
  if (key_down)
  {
    if (!world->plr.move)
    {
      world->plr.dx = 0;
    }
    world->plr.dy = 1;
    world->plr.move = 1;
  }
  else if (key_up)
  {
    if (!world->plr.move)
    {
      world->plr.dx = 0;
    }
    world->plr.dy = -1;
    world->plr.move = 1;
  }
  return orig_dx != world->plr.dx || orig_dy != world->plr.dy;
}

void handle_weapon_change_keys(World *world, int key_x, int key_z)
{
  const int difficulty = GET_DIFFICULTY(world);
  int play_sample = 0;
  if (key_x && world->plr.reload == 0) // Power
  {
    world->plr.shots = 6;
    world->plr.rate = 200;
    if (difficulty == DIFFICULTY_BRUTAL)
    {
      // better damage output but enemies are stronger
      world->plr.shots = 4;
      world->plr.rate = 30;
    }
    world->plr.reload = 20;
    play_sample = 1;
  }
  else if (key_z && world->plr.reload == 0) // Speed
  {
    world->plr.shots = 1;
    world->plr.rate = 7;
    if (difficulty == DIFFICULTY_BRUTAL)
    {
      world->plr.rate = 12;
    }
    world->plr.reload = 20;
    play_sample = 1;
  }
  if (play_sample)
    trigger_sample_with_params(SAMPLE_SELECT_WEAPON, 127, 127, 1000);
}

static inline Enemy *create_turret(World *world)
{
  Enemy *enm = ns_spawn_enemy(world->plr.x, world->plr.y, 9, world->current_room, world);
  enm->ammo = 128;
  enm->rate = 0;
  enm->shots = 2;
  enm->reload = 10;
  enm->move = 10;
  enm->dx = world->plr.dx;
  enm->dy = world->plr.dy;
  enm->health = 20;
  enm->gold = 0;
  enm->turret = TURRET_TYPE_PLAYER;
  enm->hurts_monsters = 1;
  enm->alive = 1;
  enm->killed = 0;
  return enm;
}

void handle_power_up_keys(World *world, int key_a, int key_s, int key_d, int key_f, int *gold_hint_amount)
{
  const int price_bonus = (*world->game_modifiers & GAMEMODIFIER_OVERPRICED_POWERUPS) != 0 ? 2 : 0;
  const int cost_heal = 1 + price_bonus;
  const int cost_protection = 2 + price_bonus;
  const int difficulty = GET_DIFFICULTY(world);
  const int cost_turret = (difficulty == DIFFICULTY_BRUTAL ? 4 : 3) + price_bonus;
  const int cost_blast = (difficulty == DIFFICULTY_BRUTAL ? 8 : 6) + price_bonus;

  const int overpowered = (*world->game_modifiers & GAMEMODIFIER_OVERPOWERED_POWERUPS) != 0;

  int *plr_rune_of_protection_active = &world->powerups.rune_of_protection_active;

  int max_health = world->plr_max_health;
  if (overpowered)
    max_health *= 3;
  if (key_a && world->plr.gold >= cost_heal && world->plr.health > 0 && world->plr.health < max_health && world->plr.reload == 0)
  {
    *gold_hint_amount = cost_heal;
    world->plr.gold -= cost_heal;
    int health_bonus = difficulty == DIFFICULTY_BRUTAL ? 2 : 3;
    if (world->plr.perks & PERK_IMPROVE_HEALTH_POWERUP)
      health_bonus++;
    world->plr.health += health_bonus;
    if (world->plr.health > world->plr_max_health)
    {
      world->plr.health = world->plr_max_health;
    }
    if (overpowered)
    {
      world->plr.health *= 3;
    }
    world->plr.reload = 40;
    trigger_sample(SAMPLE_HEAL, 255);
    return;
  }
  if (key_s && world->plr.gold >= cost_protection && world->plr.reload == 0 && *plr_rune_of_protection_active == 0)
  {
    *gold_hint_amount = cost_protection;
    world->plr.gold -= cost_protection;
    *plr_rune_of_protection_active = 1;
    if (world->plr.perks & PERK_IMPROVE_SHIELD_POWERUP)
      *plr_rune_of_protection_active = 3;
    world->plr.reload = 40;
    trigger_sample(SAMPLE_PROTECTION, 255);
    return;
  }
  if (key_d && world->plr.gold >= cost_turret && world->plr.reload == 0) // Turret
  {
    *gold_hint_amount = cost_turret;
    create_turret(world);
    if (overpowered)
    {
      Enemy *created = create_turret(world);
      created->dx *= 2;
      created->dy *= 2;
    }

    world->plr.gold -= cost_turret;
    world->plr.reload = 40;
    trigger_sample(SAMPLE_TURRET, 255);
    return;
  }
  if (key_f && world->plr.gold >= cost_blast && world->plr.reload == 0)
  {
    Bullet *b = get_next_available_bullet(world);
    int did_shoot = shoot_one_shot_at_xy(world->plr.x, world->plr.y, world->plr.dx, world->plr.dy, &world->plr, 1, world);
    if (did_shoot)
    {
      *gold_hint_amount = cost_blast;
      b->bullet_type = BULLET_TYPE_CLUSTER;
      b->dx *= 0.5;
      b->dy *= 0.5;
      if (overpowered)
      {
        create_cluster_explosion(world, world->plr.x, world->plr.y, 16, 16, &world->plr);
      }
      world->plr.gold -= cost_blast;
      world->plr.reload = 40;
      trigger_sample_with_params(SAMPLE_BLAST, 255, 127, 1000);
      return;
    }
  }
}

void handle_shoot_key(World *world, int key_space)
{
  if (key_space)
  {
    int play_sample = shoot(&world->plr, world);
    if (play_sample)
      trigger_sample(SAMPLE_THROW, 255);
  }
}
