#ifndef __COMMON_H
#define __COMMON_H

#include <stdint.h>
#include <inttypes.h>
#include <memory>
#include <boost/crc.hpp>
#include "consts.h"

#define DEFAULT_GAME_PORT 12345
#define DEFAULT_USER_PORT 12346
#define DEFAULT_SERVER_NAME "localhost"

#define PORT_MAX 65535

#define DEFAULT_WIDTH 800
#define DEFAULT_HEIGHT 600
#define DEFAULT_ROUNDS_PER_SEC 50
#define DEFAULT_TURNING_SPEED 6

/* W rzeczywistości maksymalna długość pola danych UDP jest trochę mniejsza,
ale te kilkadziesiąt dodatkowych bajtów nie zawadzi zaalokować. */
#define UPD_PACKET_MAX 512

uint32_t crc(char *str, uint32_t len) {
    boost::crc_32_type result;
    result.process_bytes(str, len);
    return result.checksum();
}

struct message {
  uint64_t timestamp;
  uint8_t  character;
} __attribute__((packed));

bool is_port_valid(int d) {
 return (d <= 0 || d >= PORT_MAX) ? false : true;
}

bool is_turning_speed_valid(int d) {
 return (d <= 0 || d > TURNING_SPEED_MAX) ? false : true;
}
bool is_rounds_per_sec_valid(int d) {
  return (d <= 0 || d > ROUNDS_PER_SEC_MAX) ? false : true;
}
bool is_random_seed_valid(unsigned long d) {
  return (d <= 0) ? false : true;
}
bool is_height_or_width_valid(int d) {
  return (d <= 0 || d > HEIGHT_OR_WIDTH_MAX) ? false : true;
}

struct message_from_client_to_server {
  uint64_t session_id;
  int8_t   turn_direction;
  uint32_t next_expected_event_no;
  uint8_t  player_name[MAX_PLAYER_NAME_SIZE + 1];
} __attribute__((packed));

struct event {
  uint32_t  len;
  uint32_t  event_no;
  uint8_t   event_type;
  uint8_t   event_data[MAX_EVENT_DATA];
  uint32_t  crc32;
} __attribute__((packed));

struct message_from_server_to_client {
  uint32_t  game_id;
  struct event events[MAX_NUMBER_OF_EVENTS];
} __attribute__((packed));

class GameState;
class Event;
class Player;

typedef typename std::shared_ptr<Event> EventShared;
typedef typename std::shared_ptr<GameState> GameStatePtr;
typedef typename std::shared_ptr<Player> SharedPlayer;


// bool check_player_name(struct player_name* pn) {
//   bool result = true;
//   std::cout << pn->name << std::endl;
//   std::cout << sizeof(pn) << std::endl;
//   return result;
// }

bool proper_player_name(char *pn) {
  printf("%d\n", (int)sizeof(*pn));
  return true;
}
bool is_digits(const std::string &str)
{
    return str.find_first_not_of("0123456789") == std::string::npos;
}

void increment(uint32_t &e) {
  e = (e + 1) % MAX_NUMBER_OF_EVENTS;
}

// /* Koniec roku 4242 */
// static const uint64_t TIMESTAMP_MAX = 71728934399ULL;

#endif
