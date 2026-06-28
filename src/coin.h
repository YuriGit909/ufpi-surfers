#ifndef COIN_H
#define COIN_H

void initCoins();
void updateCoins(float speed, float score);
void drawCoins();
void checkCoinCollision();

void spawnCoinArc(float x, float z);
void spawnCoinArcLow(float x, float z); // arco baixo 

int getRuCoins();

#endif