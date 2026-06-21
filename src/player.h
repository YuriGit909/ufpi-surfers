#ifndef PLAYER_H
#define PLAYER_H

void drawPlayer();
void movePlayerLeft();
void movePlayerRight();
void jump();
void updatePlayer();

float getPlayerX();
float getPlayerY();
void roll();
bool isRolling();

#endif