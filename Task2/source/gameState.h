/* Klasa stan gry i dostępne metody na potrzeby zadania z Sieci komputrowych. */
/* 7.06.2017 Marcin Michorzewski*/
#ifndef GAMESTATE_H
#define GAMESTATE_H

// #include <iostream>
#include <memory>
#include "consts.h"
#include "common.h"
#include "randomInt.h"
#include "player.h"
#include "event.h"


class GameState{
  private:
    bool isValid(uint32_t x, uint32_t y);
    uint32_t no_of_alive_players;
    void kickInactivePlayer(uint32_t nr);
  public:
    uint32_t event_no_last;
    uint32_t last_game_id;
    uint32_t maxx;
    uint32_t maxy;
    uint32_t game_id;
    uint32_t no_of_players;
    uint32_t no_of_active_players;
    bool gameStarted = false;
    std::chrono::time_point<std::chrono::high_resolution_clock> roundStart = std::chrono::high_resolution_clock::now();
    std::chrono::duration<long int, std::ratio<1l, 1000000000l> > elapsed = std::chrono::high_resolution_clock::now() - roundStart;
    unsigned long long passed_microseconds;
    int roundsPassed = 0;
    SharedPlayer players[MAX_NUMBER_OF_PLAYERS];
    bool pixelsTaken[MAX_BOARD_SIZE_X][MAX_BOARD_SIZE_Y];
    bool test() const;
    unsigned int test_game_id() const;
    GameState(EventShared *events,
              uint32_t &event_no,
              uint32_t _maxx,
               uint32_t _maxy,
               SharedPlayer *_players,
               uint8_t _no_of_players);
    void generateEvent(EventShared *events,
                       uint32_t &event_no,
                       uint8_t type,
                       uint8_t nr_of_player);
    void take(EventShared *events, uint32_t &event_no, uint8_t it);
    void makeTurn(EventShared *events, uint32_t &event_no, uint32_t turning_speed);
    void makeTurnForPlayer(EventShared *events,
                           uint32_t &event_no,
                           uint8_t player,
                           uint32_t turning_speed);
    void startGame(EventShared *events, uint32_t &event_no);
    void makeTurnOrStartGame(EventShared *events,
                             uint32_t &event_no,
                             int rounds_per_sec,
                             uint32_t turning_speed);
    void addPlayer(SharedPlayer p);
    void endGame(EventShared *events, uint32_t &event_no);
    void kickInactivePlayers();
};

#endif /* GAMESTATE_H */
