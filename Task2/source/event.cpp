/* Implementacja event.h */
/* 7.06.2017 Marcin Michorzewski*/

#include "event.h"
#include <stdio.h>
#include <string.h>
#include <cassert>


unsigned int Event::event_no_counter = 0;

Event::Event(uint8_t type, GameState *gs, uint8_t player_nr) : event_type(type) {
  assert(type >= 0 && type <= 3);
  if (event_type == 0) event_no_counter = 0;
  event_no = event_no_counter++;
  uint8_t zero = 0;

  uint32_t itd = 0;
  char *buffer = (char *) malloc(MAX_UDP_FROM_SERVER_TO_CLIENT);

  if (event_type == 0) {
    SharedPlayer p;
    uint32_t maxX = htobe32(gs->maxx), maxY = htobe32(gs->maxy);
    memcpy(buffer + itd, &maxX, 4);
    itd += 4;
    memcpy(buffer + itd, &maxY, 4);
    itd += 4;
    for (unsigned int i = 0; i < gs->no_of_active_players; ++i) {
      p = gs->players[i];
      memcpy(buffer + itd, p->name, p->name_size());
      itd += p->name_size();
      memcpy(buffer + itd, &zero, 1);
      itd += 1;
    }
  }

  else if (event_type == 1) {
    uint32_t x = htobe32(gs->players[player_nr]->x_i());
    uint32_t y = htobe32(gs->players[player_nr]->y_i());
    memcpy(buffer + itd, &player_nr, 1);
    itd += 1;
    memcpy(buffer + itd, &x, 4);
    itd += 4;
    memcpy(buffer + itd, &y, 4);
    itd += 4;
  }

  else if (event_type == 2) {
    memcpy(buffer + itd, &player_nr, 1);
    itd += 1;
  }

  // event_type == 3
  else {
  }

  this->len = itd + 5;
  event_data = (char *) malloc (sizeof(char) * itd);
  memcpy(event_data, buffer, itd);
}


char *Event::event_message() {
  if (!(event_type >= 0 && event_type <=3)) {
    return NULL;
  }
  char *result = (char *) malloc(sizeof(char) * this->give_full_length());
  uint32_t _len = htobe32(this->len);
  uint32_t _event_no = htobe32(this->event_no);
  unsigned int it = 0;
  memcpy(result + it, &_len, 4);
  it += 4;
  memcpy(result + it, &_event_no, 4);
  it += 4;
  memcpy(result + it, &(this->event_type), 1);
  it += 1;
  memcpy(result + it, (this->event_data), this->len - 5);
  it += (this->len - 5);

  uint32_t crc32 = crc(result, this->len + 4);
  crc32 = htobe32(crc32);
  memcpy(result + it, &crc32, 4);
  it += 4;

  return result;
}

uint32_t Event::give_full_length() {
  return this->len + 8;
}
