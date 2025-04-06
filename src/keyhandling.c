#include "keyhandling.h"
#include <stdio.h>
#include "duConstants.h"
#include "worldInteraction.h"
#include "sampleRegister.h"
#include "vfx.h"

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
  if (*world->game_modifiers & GAMEMODIFIER_UBER_WIZARD)
  {
    if (world->plr.reload != 0 || (!key_x && !key_z))
      return;
    if (key_x)
    {
      world->plr.reload = 20;
      world->plr.shots++;
      if (world->plr.shots > 4)
        world->plr.shots = 1;
    }
    if (key_z)
    {
      world->plr.reload = 20;
      world->plr.shots--;
      if (world->plr.shots < 1)
        world->plr.shots = 4;
    }
    trigger_sample_with_params(SAMPLE_SELECT_WEAPON, 127, 127, 1000);
    return;
  }
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

int handle_power_up_keys(World *world, int key_a, int key_s, int key_d, int key_f)
{
  if (*world->game_modifiers & GAMEMODIFIER_UBER_WIZARD)
  {
    int orig = world->plr.shots;
    if (key_a)
      world->plr.shots = 1;
    else if (key_s)
      world->plr.shots = 2;
    else if (key_d)
      world->plr.shots = 3;
    else if (key_f)
      world->plr.shots = 4;
    if (orig != world->plr.shots)
    {
      world->plr.reload = 20;
      trigger_sample_with_params(SAMPLE_SELECT_WEAPON, 127, 127, 1000);
    }
    return 0;
  }

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
    return cost_heal;
  }
  if (key_s && world->plr.gold >= cost_protection && world->plr.reload == 0 && *plr_rune_of_protection_active == 0)
  {
    world->plr.gold -= cost_protection;
    *plr_rune_of_protection_active = 1;
    if (world->plr.perks & PERK_IMPROVE_SHIELD_POWERUP)
      *plr_rune_of_protection_active = 3;
    world->plr.reload = 40;
    trigger_sample(SAMPLE_PROTECTION, 255);
    return cost_protection;
  }
  if (key_d && world->plr.gold >= cost_turret && world->plr.reload == 0) // Turret
  {
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
    return cost_turret;
  }
  if (key_f && world->plr.gold >= cost_blast && world->plr.reload == 0)
  {
    Bullet *b = get_next_available_bullet(world);
    int did_shoot = shoot_one_shot_at_xy(world->plr.x, world->plr.y, world->plr.dx, world->plr.dy, &world->plr, 1, world);
    if (did_shoot)
    {
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
      return cost_blast;
    }
  }
  return 0;
}

void handle_uber_wizard_weapon(World *world)
{
  if (world->plr.reload > 0)
    return;
  if (world->plr.shots == 4)
  {
    int *plr_rune_of_protection_active = &world->powerups.rune_of_protection_active;
    if (*plr_rune_of_protection_active != 0)
      return;
    *plr_rune_of_protection_active = 1;
    if (world->plr.perks & PERK_IMPROVE_SHIELD_POWERUP)
      *plr_rune_of_protection_active = 3;
    world->plr.reload = 200;
    trigger_sample(SAMPLE_PROTECTION, 255);
    return;
  }
  if (world->plr.dx != 0 || world->plr.dy != 0)
  {
    world->plr.reload = 200;
    double x = world->plr.x - world->plr.dx * HALFTILESIZE;
    double y = world->plr.y - world->plr.dy * HALFTILESIZE;
    int count = 0;
    while (x > 0 && !get_tile_at(world, x, y)->is_wall)
    {
      x += world->plr.dx;
      y += world->plr.dy;
      count++;
      for (int i = 0; i < ENEMYCOUNT; i++)
      {
        Enemy *enm = &world->enm[i];
        if (!enm->alive || enm->roomid != world->current_room)
          continue;
        if (enm->x - HALFTILESIZE < x && enm->x + HALFTILESIZE > x &&
            enm->y - HALFTILESIZE < y && enm->y + HALFTILESIZE > y)
        {
          create_uber_wizard_weapon_fx(world, x, y, 0);
          if (world->plr.shots == 1)
          {
            kill_enemy(enm, world);
            trigger_sample(SAMPLE_BLAST, 200);
          }
          if (world->plr.shots == 2)
          {
            for (x = -2; x <= 2; x++)
            {
              for (y = -2; y <= 2; y++)
              {
                int tx = enm->x + x * TILESIZE;
                int ty = enm->y + y * TILESIZE;
                int xx = enm->x;
                int yy = enm->y;
                while (xx != tx && yy != ty && !get_tile_at(world, xx, yy)->is_wall)
                {
                  if (tx > enm->x)
                    xx++;
                  else if (tx < enm->x)
                    xx--;
                  if (ty > enm->y)
                    yy++;
                  else if (ty < enm->y)
                    yy--;
                }
                if (xx == tx && yy == ty)
                {
                  create_cluster_explosion(world, enm->x + x * TILESIZE, enm->y + y * TILESIZE, 8, 1, &world->plr);
                }
              }
            }
            create_cluster_explosion(world, enm->x, enm->y, 16, 3, enm);
            trigger_sample(SAMPLE_WARP, 200);
          }
          if (world->plr.shots == 3)
          {
            world->plr.reload = 100;
            for (int i = 0; i < 2 && enm->health > 0; i++)
            {
              enm->health--;
              world->plr.health++;
            }
            if (world->plr.health > world->plr_max_health)
              world->plr.health = world->plr_max_health;
            if (enm->health <= 0)
            {
              kill_enemy(enm, world);
              spawn_potion(enm->x, enm->y, POTION_ID_INSTANT_HEAL, world->current_room, world, POTION_DROP_RANGE_START, POTION_DROP_RANGE_END);
            }
            trigger_sample(SAMPLE_HEAL, 200);
          }
          x = -1;
          break;
        }
      }
    }
    if (x > 0)
    {
      create_uber_wizard_weapon_fx(world, x, y, 1);
    }
  }
}

void handle_shoot_key(World *world, int key_space)
{
  if (key_space)
  {
    if (*world->game_modifiers & GAMEMODIFIER_UBER_WIZARD)
    {
      handle_uber_wizard_weapon(world);
      return;
    }
    int play_sample = shoot(&world->plr, world);
    if (play_sample)
      trigger_sample(SAMPLE_THROW, 255);
  }
}
