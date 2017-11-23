/* Moduł Event na potrzeby zadania z Sieci komputrowych. */
/* 7.06.2017 Marcin Michorzewski*/

#ifndef EVENT_H
#define EVENT_H

#include "common.h"
#include "player.h"
#include "gameState.h"

class Event{
  public:
    Event(uint8_t type, GameState *gs, uint8_t player_nr);
    Event() = delete;
    char *event_message();
    uint32_t give_full_length();
  private:
    uint8_t event_type;
    uint32_t event_no;
    uint32_t len;
    char *event_data;
    uint32_t crc32;
    static uint32_t event_no_counter;
};

#endif /* EVENT_H */
