#include "randomInt.h"

uint64_t currentRandomInt = 0;

uint64_t randomInt() {
  uint64_t result = currentRandomInt;
  currentRandomInt *= multiply;
  currentRandomInt %= modulo;
  return result;
}

void setRandomInt(uint64_t seed) {
  currentRandomInt = seed;
}
