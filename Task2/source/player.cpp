/* Implementacja player.h */
/* 7.06.2017 Marcin Michorzewski*/

#include <stdio.h>
#include <string.h>
#include <math.h>
#include "player.h"

Player::Player(){}

Player::Player(char *_name, uint32_t _size_of_name) {
  this->name = (char *) malloc (sizeof(char) * _size_of_name);
  memcpy(this->name, _name, _size_of_name);
  this->size_of_name = _size_of_name;
  x = 0;
  y = 0;
  dir = 0;
}

uint32_t Player::x_i() {
  return (uint32_t) x;
}

uint32_t Player::y_i() {
  return (uint32_t) y;
}

uint32_t Player::name_size() {
  return size_of_name;
}

void Player::setX(uint32_t max) {
  x = (long double) (randomInt() % max + 0.5);
}

void Player::setY(uint32_t max) {
  y = (long double) (randomInt() % max + 0.5);
}

void Player::setDir(uint32_t max) {
  dir = randomInt() % max;
}

void Player::setTurnDirection(int8_t _turn_direction) {
  turn_direction = _turn_direction;
}

void Player::setPosition(uint32_t maxX, uint32_t maxY) {
  setX(maxX);
  setY(maxY);
  setDir(360);
}

int8_t Player::giveTurnDirection() {
  return turn_direction;
}

void Player::addToDir(int32_t turning_speed) {
  dir += turning_speed;
  while (dir >= 360) {
    dir -= 360;
  }
  while (dir < 0) {
    dir += 360;
  }
}

void Player::moveForward() {
  x += cos(dir * PI / 180.0);
  y += sin(dir * PI / 180.0);
}
