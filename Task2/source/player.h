/* Klasa gracz i dostępne metody na potrzeba zadania z Sieci komputrowych. */
/* 7.06.2017 Marcin Michorzewski*/
#ifndef PLAYER_H
#define PLAYER_H

#include <iostream>
#include <memory>
#include <chrono>

#include "consts.h"

class Player{
  public:
    Player(char *name, uint32_t _size_of_name);
    Player();
    uint32_t x_i();
    uint32_t y_i();
    char *name;
    uint32_t name_size();
    void setPosition(uint32_t maxX, uint32_t maxY);
    void setTurnDirection(int8_t);
    int8_t giveTurnDirection();
    void addToDir(int32_t turning_speed);
    void moveForward();
    bool isAlive;
  private:
    void setX(uint32_t max);
    void setY(uint32_t max);
    void setDir(uint32_t max);
    uint32_t size_of_name;
    long double x;
    long double y;
    long double dir;
    int8_t turn_direction;
};

bool operator<(SharedPlayer const &lhs, SharedPlayer const &rhs) {
  if (lhs->name_size() == 0)
    return false;
  if (rhs->name_size() == 0)
    return true;
  uint32_t miniSize = std::min(lhs->name_size(), rhs->name_size());
  for (unsigned int i = 0; i < miniSize; ++i) {
    if (lhs->name[i] < rhs->name[i]) {
      return true;
    }
    if (lhs->name[i] > rhs->name[i]) {
      return false;
    }
  }
  return (lhs->name_size() < rhs->name_size()) ? true : false;
}

#endif /* PLAYER_H */
