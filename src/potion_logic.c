#include "potion_logic.h"
#include "worldInteraction.h"
#include "sprites.h"
#include "sampleRegister.h"
#include <math.h>

void potion_logic(World *w)
{
  static unsigned potion_anim_phase = 0;
  for (int i = 0; i < POTION_COUNT; i++)
  {
    Potion *p = &w->potions[i];
    if (p->exists && p->room_id == w->current_room)
    {
      if (w->plr.x > p->location.x - 20 && w->plr.x < p->location.x + 20 &&
          w->plr.y > p->location.y - 20 && w->plr.y < p->location.y + 20 && w->plr.health > 0)
      {
        p->exists = 0;
        w->potion_duration += p->duration_boost;
        if (w->potion_duration > POTION_DURATION_CAP)
          w->potion_duration = POTION_DURATION_CAP;
        if ((p->effects & POTION_EFFECT_HEAL_ONCE) && w->plr.health < w->plr_max_health)
        {
          w->plr.health = w->plr_max_health;
        }
        if ((w->game_modifiers & GAMEMODIFIER_POTION_ON_DEATH) && (w->potion_effect_flags & p->effects) && w->potion_duration == POTION_DURATION_CAP)
        {
          create_cluster_explosion(w, w->plr.x, w->plr.y, 8, 2, &w->plr);
        }
        w->potion_effect_flags |= p->effects;
        trigger_sample(p->sample, 255);
        create_sparkles(p->location.x, p->location.y, 12, 1, 10, w);
      }

      unsigned anim_phase = (potion_anim_phase + i * 1337) % 200;
      if (anim_phase < 30)
      {
        int bubble_y = anim_phase / 5;
        int bubble_x = 4 * sin(bubble_y);
        int pop_spr = anim_phase > 25;
        draw_sprite_animated_centered(w->spr, SPRITE_ID_POTION_BUBBLE, p->location.x + bubble_x, p->location.y - 13 - bubble_y, p->sprite * 2 + pop_spr, 0);
      }
      draw_sprite_animated_centered(w->spr, SPRITE_ID_POTION, p->location.x, p->location.y, p->sprite, 0);
    }
  }
  potion_anim_phase++;
  if (w->potion_duration > 0)
  {
    w->potion_duration -= w->potion_turbo_mode ? 2 : 1;
  }
  if (w->potion_duration <= 0)
  {
    w->potion_effect_flags = 0;
  }
}