#ifndef OBSTACLE_H
#define OBSTACLE_H

void initObstacles();
void drawObstacles();
void updateObstacles(float speed, float score);
void checkCollision();
bool canMoveToLane(float targetX);
void initObstacleModels();

#endif