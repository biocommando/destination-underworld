#include "keyhandling.h"
#include <stdio.h>
#include "gamePersistence.h"
#include "duConstants.h"
#include "worldInteraction.h"
#include "sampleRegister.h"

//extern struct gamedata UniqueGameData;

void getKeyPresses(ContinuousData *data, void *keyPressOutput)
{
     long *keyPressOutputLong = (long *) keyPressOutput;
     if (data->dataTypeId == 1)
     {
       *keyPressOutputLong = data->dataValue;
     }
}

int handleDirectionKeys(World *world, int keyUp, int keyDown, int keyLeft, int keyRight)
{
        int orig_dx = world->plr.dx;
        int orig_dy = world->plr.dy;
        world->plr.move = 0;
        if (keyLeft)
        {
            world->plr.dx = -1;
            world->plr.dy = 0;
            world->plr.move = 1;
        }
        else if (keyRight)
        {
            world->plr.dx = 1;
            world->plr.dy = 0;
            world->plr.move = 1;
        }
        if (keyDown)
        {
            if (!world->plr.move)
            {
              world->plr.dx = 0;
            }
            world->plr.dy = 1;
            world->plr.move = 1;
        }
        else if (keyUp)
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

int handleWeaponChangeKeys(World *world, int keyX, int keyZ)
{
    const int difficulty = (world->gameModifiers & GAMEMODIFIER_BRUTAL) != 0;
    if (keyX && world->plr.wait == 0) // Power
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
    if (keyZ && world->plr.wait == 0) // Speed
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

Enemy *createTurret(World *world)
{
  int enmIdx = 0;
  Enemy *enm = getNextAvailableEnemy(world, &enmIdx);
  enm->id = 9000 + enmIdx;
  enm->formerId = enm->id;
  enm->ammo = 128;
  enm->x = world->plr.x;
  enm->y = world->plr.y;
  enm->rate = 1;
  enm->shots = 2;
  enm->reload = 10;
  enm->move = 10;
  enm->dx = world->plr.dx;
  enm->dy = world->plr.dy;
  enm->health = 20;
  enm->roomid = world->currentRoom;
  enm->gold = 0;
  enm->type = TURRET;
  return enm;
}

int handlePowerUpKeys(World *world, int keyA, int keyS, int keyD, int keyF, int *goldHintAmount, int *plr_rune_of_protection_active)
{
    const int price_bonus = (world->gameModifiers & GAMEMODIFIER_OVERPRICED_POWERUPS) != 0 ? 2 : 0;
    const int cost_heal = 1 + price_bonus;
    const int cost_protection = 2 + price_bonus;
    const int difficulty = (world->gameModifiers & GAMEMODIFIER_BRUTAL) != 0 ? DIFFICULTY_BRUTAL : 0;
    const int cost_turret = (difficulty == DIFFICULTY_BRUTAL ? 4 : 3) + price_bonus;
    const int cost_blast = (difficulty == DIFFICULTY_BRUTAL ? 8 : 6) + price_bonus;
    
    const int overpowered = (world->gameModifiers & GAMEMODIFIER_OVERPOWERED_POWERUPS) != 0;
    
    if (keyA && world->plr.gold >= cost_heal && world->plr.health > 0 && world->plr.health < 6 && world->plr.wait == 0)
    {
      //UniqueGameData.powerups++;
      *goldHintAmount = cost_heal;
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
      triggerSample(SAMPLE_HEAL, 255);
      return 1;
    }
    if (keyS && world->plr.gold >= cost_protection && world->plr.wait == 0 && *plr_rune_of_protection_active == 0)
    {
      //UniqueGameData.powerups++;
      *goldHintAmount = cost_protection;
      world->plr.gold -= cost_protection;
      *plr_rune_of_protection_active = 1;
      world->plr.reload = 40;
      world->plr.wait = 80;
      triggerSample(SAMPLE_PROTECTION, 255);
      return 2;
    }
    if (keyD && world->plr.gold >= cost_turret && world->plr.wait == 0) // Turret
    {
      //UniqueGameData.powerups++;
      *goldHintAmount = cost_turret;
      createTurret(world);
      if (overpowered)
      {
          Enemy *created = createTurret(world);
          created->dx *= 2;
          created->dy *= 2;
      }
      
      world->plr.gold -= cost_turret;
      world->plr.reload = 40;
      world->plr.wait = 80;
      triggerSample(SAMPLE_TURRET, 255);
      return 3;
    }
    if (keyF && world->plr.gold >= cost_blast && world->plr.wait == 0)
    {
      //UniqueGameData.powerups++;
      Bullet *b = getNextAvailableBullet(world);
      int didShoot = shootOneShotAtXy(world->plr.x, world->plr.y, world->plr.dx, world->plr.dy, PLAYER_ID, 1, world);
      if (didShoot)
      {
          *goldHintAmount = cost_blast;
          b->bulletType = BULLET_TYPE_CLUSTER;
          b->dx *= 0.5;
          b->dy *= 0.5;
          if (overpowered)
          {
             createClusterExplosion(world, world->plr.x, world->plr.y, 16, 16, PLAYER_ID);
          }
          world->plr.gold -= cost_blast;
          world->plr.reload = 40;
          world->plr.wait = 80;
          triggerSampleWithParams(SAMPLE_BLAST, 255, 127, 1000);
          return 4;
      }
    }
    return 0;
}

int handleShootKey(World *world, int keySpace)
{
    if (keySpace)
    {
      return shoot(&world->plr, world);
    }
    return 0;
}
