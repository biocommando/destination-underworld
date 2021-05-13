#include "keyhandling.h"
#include <stdio.h>
#include "gamePersistence.h"
#include "duConstants.h"
#include "worldInteraction.h"
#include "sampleRegister.h"

void get_key_presses(ContinuousData *data, void *key_press_output)
{
     long *key_press_output_long = (long *) key_press_output;
     if (data->data_type_id == 1)
     {
       *key_press_output_long = data->data_value;
     }
}

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

int handle_weapon_change_keys(World *world, int key_x, int key_z)
{
    const int difficulty = GET_DIFFICULTY(world);
    if (key_x && world->plr.wait == 0) // Power
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
      world->plr.wait = 20;
      return 1;
    }
    if (key_z && world->plr.wait == 0) // Speed
    {
      world->plr.shots = 1;
      world->plr.rate = 7;
      if (difficulty == DIFFICULTY_BRUTAL)
      {
          world->plr.rate = 12;
      }
      world->plr.reload = 20;
      world->plr.wait = 20;
      return 2;
    }
    return 0;
}

Enemy *create_turret(World *world)
{
  Enemy *enm = ns_spawn_enemy(world->plr.x, world->plr.y, 9, world->current_room, world);
  enm->ammo = 128;
  enm->rate = 1;
  enm->shots = 2;
  enm->reload = 10;
  enm->move = 10;
  enm->dx = world->plr.dx;
  enm->dy = world->plr.dy;
  enm->health = 20;
  enm->gold = 0;
  enm->turret = 2;
  enm->hurts_monsters = 1;
  enm->id += 1000;
  enm->former_id += 1000;
  return enm;
}

int handle_power_up_keys(World *world, int key_a, int key_s, int key_d, int key_f, int *gold_hint_amount)
{
    const int price_bonus = (world->game_modifiers & GAMEMODIFIER_OVERPRICED_POWERUPS) != 0 ? 2 : 0;
    const int cost_heal = 1 + price_bonus;
    const int cost_protection = 2 + price_bonus;
    const int difficulty = GET_DIFFICULTY(world);
    const int cost_turret = (difficulty == DIFFICULTY_BRUTAL ? 4 : 3) + price_bonus;
    const int cost_blast = (difficulty == DIFFICULTY_BRUTAL ? 8 : 6) + price_bonus;
    
    const int overpowered = (world->game_modifiers & GAMEMODIFIER_OVERPOWERED_POWERUPS) != 0;

    int *plr_rune_of_protection_active = &world->powerups.rune_of_protection_active;
    
    if (key_a && world->plr.gold >= cost_heal && world->plr.health > 0 && world->plr.health < (overpowered ? 18 : 6) && world->plr.wait == 0)
    {
      *gold_hint_amount = cost_heal;
      world->plr.gold -= cost_heal;
      world->plr.health += difficulty == DIFFICULTY_BRUTAL ? 2 : 3;
      if (world->plr.health > 6)
      {
        world->plr.health = 6;
      }
      if (overpowered)
      {
        world->plr.health *= 3;
      }
      world->plr.reload = 40;
      world->plr.wait = 80;
      trigger_sample(SAMPLE_HEAL, 255);
      return 1;
    }
    if (key_s && world->plr.gold >= cost_protection && world->plr.wait == 0 && *plr_rune_of_protection_active == 0)
    {
      *gold_hint_amount = cost_protection;
      world->plr.gold -= cost_protection;
      *plr_rune_of_protection_active = 1;
      world->plr.reload = 40;
      world->plr.wait = 80;
      trigger_sample(SAMPLE_PROTECTION, 255);
      return 2;
    }
    if (key_d && world->plr.gold >= cost_turret && world->plr.wait == 0) // Turret
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
      world->plr.wait = 80;
      trigger_sample(SAMPLE_TURRET, 255);
      return 3;
    }
    if (key_f && world->plr.gold >= cost_blast && world->plr.wait == 0)
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
          world->plr.wait = 80;
          trigger_sample_with_params(SAMPLE_BLAST, 255, 127, 1000);
          return 4;
      }
    }
    return 0;
}

int handle_shoot_key(World *world, int key_space)
{
    if (key_space)
    {
      return shoot(&world->plr, world);
    }
    return 0;
}
