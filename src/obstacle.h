#ifndef OBSTACLE_H
#define OBSTACLE_H

void initObstacles();
void drawObstacles();
void updateObstacles(float speed, float score);
void checkCollision();
bool canMoveToLane(float targetX);
void initObstacleModels();
bool hasObstacleNear(float x, float z, float radiusX, float radiusZ);
void clearNearbyObstacles(float z);

// IA 2 – chamados de player.cpp ao pular/rolar
void profileRegisterJump();
void profileRegisterRoll();

#endif
