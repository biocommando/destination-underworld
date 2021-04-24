#ifndef WORLDINTERACTION_H
#define WORLDINTERACTION_H
#include "world.h"

void move_enemy(Enemy *enm, World *world);
void createShadeAroundHitPoint(int x, int y, int spread, World *world);
void move_bullet(Bullet *bb, World *world);
Bullet *getNextAvailableBullet(World *world);
int shoot(Enemy *enm, World *world);
int shootOneShotAtXy(double x, double y, double dx, double dy, int enm_id, int hurtsMonsters, World *world);
int bullet_hit(Enemy *enm, Bullet *bb);
int sees_each_other(Enemy *e1, Enemy *e2, World *world);
void createExplosion(int x, int y, World *world);
void readLevel(World *world, const char *missionName, int roomTo);
Enemy *getNextAvailableEnemy(World *world, int *index);
Enemy *spawnEnemy(int x, int y, int type, int roomId, World *world);
void createClusterExplosion(World *w, double x0, double y0, int intensity);
void changeRoomIfAtExitPoint(World *world, int mission);

#endif
