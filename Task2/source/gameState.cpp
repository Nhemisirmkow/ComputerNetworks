/* Implementacja gameState.h */
/* 7.06.2017 Marcin Michorzewski*/

#include "gameState.h"

bool GameState::test() const{
  return true;
}

void GameState::kickInactivePlayer(uint32_t nr) {
  swap(players[no_of_players - 1], players[nr]);
  players[no_of_players - 1] = std::make_shared<Player>();
  no_of_players--;
}

GameState::GameState(EventShared *events,
                     uint32_t &event_no,
                     uint32_t _maxx,
                     uint32_t _maxy,
                     SharedPlayer *_players,
                     uint8_t _no_of_players) :
  maxx(_maxx), maxy(_maxy) {

  for (unsigned int i = 0; i < maxx; ++i) {
    for (unsigned int o = 0; o < maxy; ++o) {
      pixelsTaken[i][o] = 0;
    }
  }

  no_of_players = _no_of_players;
  for (uint8_t i = 0; i < no_of_players; ++i) {
    players[i] = _players[i];
    players[i]->isAlive = true;
  }

  if (no_of_players > 1) {
    startGame(events, event_no);
  }
}

void GameState::kickInactivePlayers() {
  for (unsigned int i = 0; i < no_of_players; ++i) {
    if (players[i].use_count() <= 1) {
      kickInactivePlayer(i);
    }
  }
}

void GameState::startGame(EventShared *events, uint32_t &event_no) {

  kickInactivePlayers();

  sort(players, players + no_of_players);

  no_of_active_players = no_of_players;

  for(unsigned int i = 0; i < no_of_players; ++i) {
    if (players[i]->name_size() == 0) {
      no_of_active_players--;
    }
  }

  event_no = 0;
  no_of_alive_players = no_of_active_players;
  gameStarted = true;
  roundStart = std::chrono::high_resolution_clock::now();
  elapsed = std::chrono::high_resolution_clock::now() - roundStart;
  roundsPassed = 1;
  game_id = randomInt();

  for (unsigned int i = 0; i < maxx; ++i) {
    for (unsigned int o = 0; o < maxy; ++o) {
      pixelsTaken[i][o] = 0;
    }
  }

  generateEvent(events, event_no, 0, 0);

  for (uint8_t i = 0; i < no_of_active_players; ++i) {
    // Now observators begins.
    if (players[i]->name_size() == 0) {
      no_of_alive_players = i;
      break;
    }
    players[i]->setPosition(maxx, maxy);
    if (!isValid(players[i]->x_i(), players[i]->y_i())) {
      players[i]->isAlive = false;
      generateEvent(events, event_no, 2, i);
    }
    else {
      players[i]->isAlive = true;
      take(events, event_no, i);
    }
  }
  if (no_of_alive_players <= 1) {
    endGame(events, event_no);
  }
}

bool GameState::isValid(uint32_t x, uint32_t y) {
  if ((x >= 0 && x < maxx) && (y >= 0 && y < maxy) && !pixelsTaken[x][y])
    return true;
  return false;
}

// NOT SAFE - CHECK IF IS FIELD VALID FIRST
void GameState::take(EventShared *events, uint32_t &event_no, uint8_t i) {
  pixelsTaken[players[i]->x_i()][players[i]->y_i()] = 1;
  generateEvent(events, event_no, 1, i);
}

void GameState::generateEvent(EventShared *events,
                              uint32_t &nr,
                              uint8_t type,
                              uint8_t nr_of_player) {
  events[nr] = std::make_shared<Event>(Event(type, this, nr_of_player));
  increment(nr);
}

unsigned int GameState::test_game_id() const {
  return game_id;
}

void GameState::makeTurn(EventShared *events,
                         uint32_t &event_no,
                         uint32_t turning_speed) {
  for (uint8_t i = 0; i < no_of_players; ++i) {
    if (players[i]->isAlive)
      makeTurnForPlayer(events, event_no, i, turning_speed);
  }
  roundsPassed++;
  if (no_of_alive_players <= 1) {
    endGame(events, event_no);
  }
}

void GameState::makeTurnForPlayer(EventShared *events,
                                  uint32_t &event_no,
                                  uint8_t i,
                                  uint32_t turning_speed) {
  if (!gameStarted) {
    return;
  }
  if (players[i]->giveTurnDirection() == 1) {
    players[i]->addToDir(turning_speed);
  }
  else if (players[i]->giveTurnDirection() == -1) {
    players[i]->addToDir((int32_t)-turning_speed);
  }
  uint32_t oldX = players[i]->x_i(), oldY = players[i]->y_i();
  players[i]->moveForward();
  uint32_t newX = players[i]->x_i(), newY = players[i]->y_i();
  if (oldX != newX || oldY != newY) {
    if (!isValid(newX, newY)) {
      players[i]->isAlive = false;
      generateEvent(events, event_no, 2, i);
    }
    else {
      take(events, event_no, i);
    }
  }
  if (!players[i]->isAlive) {
    no_of_alive_players--;
  }
}

void GameState::endGame(EventShared *events, uint32_t &event_no) {
  event_no_last = event_no + 1;
  last_game_id = game_id;
  gameStarted = false;
  generateEvent(events, event_no, 3, -1);
}

void GameState::makeTurnOrStartGame(EventShared *events,
                                    uint32_t &event_no,
                                    int rounds_per_sec,
                                    uint32_t turning_speed) {
  if (gameStarted) {
    elapsed = std::chrono::high_resolution_clock::now() - roundStart;
    passed_microseconds = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
    while ((roundsPassed + 1) * MICROSECONDS_IN_SECOND < passed_microseconds * rounds_per_sec) {
      makeTurn(events, event_no, turning_speed);
    }
    if (abs(roundsPassed * MICROSECONDS_IN_SECOND - passed_microseconds * rounds_per_sec) < TIME_EPS2) {
      makeTurn(events, event_no, turning_speed);
    }
  }
  else {
    uint32_t no_of_observators = 0;
    bool canWeStart = (no_of_players > 1) ? true : false;
    for (unsigned int i = 0; i < no_of_players; ++i) {
      if (players[i]->name_size() == 0)
        no_of_observators++;
      if (players[i]->giveTurnDirection() == 0 && players[i]->name_size() != 0) {
        canWeStart = false;
        break;
      }
    }
    canWeStart = (no_of_players - no_of_observators > 1) ? canWeStart : false;
    if (canWeStart) {
      startGame(events, event_no);
    }
    else {
    }
  }
}

void GameState::addPlayer(SharedPlayer p) {
  if (no_of_players < MAX_NUMBER_OF_PLAYERS) {
    players[no_of_players] = p;
    p->isAlive = false;
    no_of_players++;
  }
}
