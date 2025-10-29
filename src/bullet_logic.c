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
#include "game_tuning.h"
#include <math.h>

static inline void create_blast_powerup_explosion(const Bullet *bullet, World *world)
{
    if (world->plr.perks & PERK_IMPROVE_BLAST_POWERUP && get_tuning_params()->blast_powerup_perk_turret_enabled)
    {
        Enemy *e = create_turret(world);
        e->x = bullet->x;
        e->y = bullet->y;
        e->dx = e->dy = 0;
    }
    create_cluster_explosion(world, bullet->x, bullet->y, 16, world->powerups.cluster_strength, &world->plr);
}

void bullet_logic(World *world, GlobalGameState *ggs)
{
    const int difficulty = GET_DIFFICULTY(world);
    Bullet *bullet;
    LINKED_LIST_FOR_EACH(&world->bullets, Bullet, bullet, bullet->owner == NULL)
    {
        double bullet_orig_x = bullet->x;
        double bullet_orig_y = bullet->y;
        for (int j = 0; j < 12; j++)
        {
            move_bullet(bullet, world);
            if (bullet->owner == NULL)
            {
                trigger_sample(SAMPLE_EXPLOSION(rand() % 6), 200);
                create_explosion(bullet_orig_x, bullet_orig_y, world, &world->visual_fx, 0.5);
                if (bullet->bullet_type == BULLET_TYPE_CLUSTER)
                {
                    bullet->x = TO_PIXEL_COORDINATES((int)(bullet_orig_x / TILESIZE));
                    bullet->y = TO_PIXEL_COORDINATES((int)(bullet_orig_y / TILESIZE));
                    while (get_wall_type_at(world, (int)bullet->x, (int)bullet->y) != 0)
                    {
                        bullet->x -= 5 * bullet->dx;
                        bullet->y -= 5 * bullet->dy;
                    }
                    create_blast_powerup_explosion(bullet, world);
                }

                break;
            }
            if (rand() % 400 == 0)
                create_flame_fx(bullet->x, bullet->y, world, &world->visual_fx);
            if (j == 0 && (world->plr.perks & PERK_IMPROVE_BLAST_POWERUP) && bullet->bullet_type == BULLET_TYPE_CLUSTER)
            {
                if (world->time_stamp % get_tuning_params()->blast_powerup_perk_timed_cluster_rate == 0)
                    create_cluster_explosion(world, bullet->x, bullet->y, 4, 1, &world->plr);
            }
            if ((bullet->hurts_flags & BULLET_HURTS_PLAYER) && bullet_hit(&world->plr, bullet)) // Player gets hit
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
                        world->visual_fx.rune_of_protection_animation = -50;
                        create_cluster_explosion(world, world->plr.x, world->plr.y, 16, difficulty == DIFFICULTY_BRUTAL ? 3 : 4, &world->plr);
                        if ((ggs->game_modifiers & GAMEMODIFIER_OVERPOWERED_POWERUPS) != 0)
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
                create_shade_around_hit_point(world->plr.x, world->plr.y, world->current_room, 9, &world->visual_fx);
                create_explosion(world->plr.x, world->plr.y, world, &world->visual_fx, 1);
                if (world->plr.health <= 0)
                {
                    world->plr.killed = 1;
                    create_explosion(world->plr.x - 20, world->plr.y - 20, world, &world->visual_fx, 1.5);
                    create_explosion(world->plr.x + 20, world->plr.y + 20, world, &world->visual_fx, 1.5);
                    create_explosion(world->plr.x - 20, world->plr.y + 20, world, &world->visual_fx, 1.5);
                    create_explosion(world->plr.x + 20, world->plr.y - 20, world, &world->visual_fx, 1.5);
                    // As player is not really an "enemy" (not in the same array), the bodypart logic is not
                    // run for player object. Copy a vacant enemy to player's place so that the death animation
                    // will play for that object instead.
                    Enemy *substitute = get_next_available_enemy(world);
                    memcpy(substitute, &world->plr, sizeof(Enemy));

                    trigger_sample_with_params(SAMPLE_DEATH(rand() % 6), 255, 127 + (world->plr.x - 240) / 8, 900 + rand() % 200);
                    world->plr.reload = 100;
                }
                break;
            }
            if (bullet->hurts_flags & BULLET_HURTS_MONSTERS)
            {
                Enemy *enm;
                LINKED_LIST_FOR_EACH(&world->enm, Enemy, enm, 0)
                {
                    if (!enm->alive || enm->turret == TURRET_TYPE_PLAYER || enm->roomid != world->current_room)
                        continue;

                    if (bullet_hit(enm, bullet))
                    {
                        create_shade_around_hit_point(enm->x, enm->y, world->current_room, 9, &world->visual_fx);
                        create_explosion(enm->x, enm->y, world, &world->visual_fx, 1.2);
                        trigger_sample(SAMPLE_EXPLOSION(rand() % 6), 200);

                        if (bullet->bullet_type == BULLET_TYPE_CLUSTER)
                        {
                            create_blast_powerup_explosion(bullet, world);
                        }

                        if (!enm->alive) // enemy was killed (bullet_hit has side effects)
                        {
                            kill_enemy(enm, world);
                        }
                        break;
                    }
                }
                if (bullet->owner == NULL)
                    break;
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
            for (unsigned j = 0; j < limit; j++)
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
