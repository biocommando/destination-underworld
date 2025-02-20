#include "bullet_logic.h"

#include "logging.h"
#include "predictableRandom.h"
#include "worldInteraction.h"
#include "renderWorld.h"
#include "sprites.h"
#include "sampleRegister.h"
#include "boss_logic.h"
#include "duColors.h"
#include "vfx.h"
#include <math.h>

void bullet_logic(World *world, GlobalGameState *ggs)
{
  const int difficulty = GET_DIFFICULTY(world);
  for (int i = 0; i < BULLETCOUNT; i++)
  {
    Bullet *bullet = &world->bullets[i];

    if (bullet->owner == NULL)
      continue;
    double bullet_orig_x = bullet->x;
    double bullet_orig_y = bullet->y;
    for (int j = 0; j < 12; j++)
    {
      move_bullet(bullet, world);
      if (bullet->owner == NULL)
      {
        trigger_sample(SAMPLE_EXPLOSION(rand() % 6), 200);
        create_explosion(bullet_orig_x, bullet_orig_y, world, 0.5);
        if (bullet->bullet_type == BULLET_TYPE_CLUSTER)
        {
          bullet->x = TO_PIXEL_COORDINATES((int)(bullet_orig_x / TILESIZE));
          bullet->y = TO_PIXEL_COORDINATES((int)(bullet_orig_y / TILESIZE));
          while (get_tile_at(world, (int)bullet->x, (int)bullet->y)->is_wall)
          {
            bullet->x -= 5 * bullet->dx;
            bullet->y -= 5 * bullet->dy;
          }
          create_cluster_explosion(world, bullet->x, bullet->y, 16, world->powerups.cluster_strength, &world->plr);
        }

        break;
      }
      int frame_cnt = bullet->y;
      if (bullet->dx > 0.2 || bullet->dx < -0.2)
        frame_cnt = bullet->x;
      if (rand() % 400 == 0)
        create_flame_fx(bullet->x, bullet->y, world);
      if ((world->plr.perks & PERK_IMPROVE_BLAST_POWERUP) && bullet->bullet_type == BULLET_TYPE_CLUSTER)
      {
        if (frame_cnt % 64 == 0)
          create_cluster_explosion(world, bullet->x, bullet->y, 4, 1, &world->plr);
      }
      if ((bullet->hurts_flags & BULLET_HURTS_PLAYER) && bullet_hit(&world->plr, world->bullets + i)) // Player gets hit
      {
        if (ggs->cheats & 1)
          world->plr.health++;
        if (world->powerups.rune_of_protection_active > 0)
        {
          world->plr.health++;
          if (world->plr.health < 0)
            world->plr.health = 1;
          world->plr.alive = 1;
          world->plr.killed = 0;
          world->powerups.rune_of_protection_active--;
          if (world->powerups.rune_of_protection_active == 0)
          {
            world->powerups.rune_of_protection_active = -50;
            create_cluster_explosion(world, world->plr.x, world->plr.y, 16, difficulty == DIFFICULTY_BRUTAL ? 3 : 4, &world->plr);
            if ((world->game_modifiers & GAMEMODIFIER_OVERPOWERED_POWERUPS) != 0)
            {
              create_cluster_explosion(world, world->plr.x, world->plr.y, 16, difficulty == DIFFICULTY_BRUTAL ? 3 : 4, &world->plr);
            }
          }
          else
          {
            create_cluster_explosion(world, world->plr.x, world->plr.y, 6, 1, &world->plr);
          }
        }
        trigger_sample(SAMPLE_EXPLOSION(rand() % 6), 200);
        create_shade_around_hit_point(world->plr.x, world->plr.y, 9, world);
        create_explosion(world->plr.x, world->plr.y, world, 1);
        if (world->plr.health <= 0)
        {
          create_explosion(world->plr.x - 20, world->plr.y - 20, world, 1.5);
          create_explosion(world->plr.x + 20, world->plr.y + 20, world, 1.5);
          create_explosion(world->plr.x - 20, world->plr.y + 20, world, 1.5);
          create_explosion(world->plr.x + 20, world->plr.y - 20, world, 1.5);
          // As player is not really an "enemy" (not in the same array), the bodypart logic is not
          // run for player object. Copy a vacant enemy to player's place so that the death animation
          // will play for that object instead.
          Enemy *substitute = get_next_available_enemy(world);
          memcpy(substitute, &world->plr, sizeof(Enemy));

          trigger_sample_with_params(SAMPLE_DEATH(rand() % 6), 255, 127 + (world->plr.x - 240) / 8, 900 + rand() % 200);
          world->plr.reload = 100;
          break;
        }
      }
      if (bullet->hurts_flags & BULLET_HURTS_MONSTERS)
      {
        for (int j = 0; j < ENEMYCOUNT; j++)
        {
          Enemy *enm = &world->enm[j];
          if (!enm->alive || enm->turret == TURRET_TYPE_PLAYER || enm->roomid != world->current_room)
            continue;

          if (bullet_hit(world->enm + j, world->bullets + i))
          {
            create_shade_around_hit_point(enm->x, enm->y, 9, world);
            create_explosion(enm->x, enm->y, world, 1.2);
            trigger_sample(SAMPLE_EXPLOSION(rand() % 6), 200);

            if (bullet->bullet_type == BULLET_TYPE_CLUSTER)
            {
              create_cluster_explosion(world, bullet->x, bullet->y, 16, world->powerups.cluster_strength, &world->plr);
            }

            if (!enm->alive) // enemy was killed (bullet_hit has side effects)
            {
              create_explosion(enm->x, enm->y, world, 1.8);

              if (check_potion_effect(world, POTION_EFFECT_STOP_ENEMIES))
              {
                world->potion_effect_flags &= ~POTION_EFFECT_STOP_ENEMIES;
                if (!world->potion_effect_flags)
                  world->potion_duration = 0;
              }

              get_tile_at(world, enm->x, enm->y)->is_blood_stained = 1;
              if ((world->game_modifiers & GAMEMODIFIER_NO_GOLD) == 0)
                world->plr.gold += enm->gold;

              world->kills++;
              world->boss_fight_config->state.player_kills++;
              world->plr.xp += enm->xp;
              if (enm->gold > 0 || (world->game_modifiers & GAMEMODIFIER_ARENA_FIGHT))
              {
                if (world->game_modifiers & GAMEMODIFIER_ARENA_FIGHT)
                  sprintf(world->hint.text, "%d", world->kills);
                else
                  sprintf(world->hint.text, "+ %d", enm->gold);
                world->hint.loc.x = enm->x - 15;
                world->hint.loc.y = enm->y - 15;
                world->hint.dim = 6;
                world->hint.time_shows = 40;
              }
              if ((world->game_modifiers & GAMEMODIFIER_MULTIPLIED_GOLD) == 0)
              {
                if (world->plr.health < 3 && world->plr.health > 0)
                  world->plr.health++;
                world->plr.ammo += 7;
                if (world->plr.ammo > 15)
                  world->plr.ammo = 15;
              }
              if (enm->potion >= POTION_ID_SHIELD)
              {
                int potion = enm->potion;
                // if almost out of health, spawn healing potion
                if (world->plr.health == 1)
                  potion = POTION_ID_INSTANT_HEAL;
                spawn_potion(enm->x, enm->y, potion, world->current_room, world, POTION_DROP_RANGE_START, POTION_DROP_RANGE_END);
              }

              if (enm == world->boss) // Archmage dies
              {
                LOG_TRACE("boss die logic\n");
                boss_logic(world, 1);
                stop_all_samples();
                trigger_sample_with_params(SAMPLE_BOSSTALK_2, 255, 127 + (enm->x - 240) / 8, 1000);
                for (int xx = 0; xx < 40; xx++)
                {
                  int col = xx % 4;
                  col = col == 0 ? 255 : (col == 2 ? 64 : 128);
                  al_draw_filled_rectangle(0, 0, SCREEN_W + 1, SCREEN_H + 1, GRAY(col));
                  wait_delay_ms(25);
                }
                create_cluster_explosion(world, enm->x, enm->y, 48, 1, &world->plr);
              }
              else
              {
                trigger_sample_with_params(SAMPLE_DEATH(rand() % 6), 255, 127 + (enm->x - 240) / 8, 900 + rand() % 200);
              }
            }
            break;
          }
        }
      }
    }
    if (bullet->bullet_type == BULLET_TYPE_NORMAL)
    {
      int bullet_sprite = ((int)(bullet->x + bullet->y) / 30) % 4;
      draw_sprite_animated_centered(world->spr, SPRITE_ID_BULLET, bullet->x, bullet->y, bullet_sprite, 0);

      // Draw bullet trail
      double dx = bullet_orig_x - bullet->x;
      double dy = bullet_orig_y - bullet->y;
      unsigned limit = bullet->duration;
      if (limit > 4)
        limit = 4;
      for (int j = 0; j < limit; j++)
      {
        draw_sprite_animated_centered(world->spr, SPRITE_ID_BULLET,
                                      bullet_orig_x + dx * j + rand() % 3 - 1, bullet_orig_y + dy * j + rand() % 3 - 1,
                                      (j + bullet_sprite) % 4, -1 - j);
      }
    }
    else if (bullet->bullet_type == BULLET_TYPE_CLUSTER)
    {
      draw_sprite_centered(world->spr, SPRITE_ID_CLUSTER, bullet->x, bullet->y);
    }
    bullet->duration++;
  }
}
