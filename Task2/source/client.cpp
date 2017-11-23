#define _DEFAULT_SOURCE
#include <string>
#include <assert.h>
#include <endian.h>
#include <inttypes.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <chrono>
#include <iostream>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>

#include "err.h"
#include "common.h"
#include "consts.h"


std::string player_names[MAX_NUMBER_OF_PLAYERS + 1];
struct message_from_client_to_server msg_to_serv;
unsigned int curr_players_nr = 0;
std::string event_data = "";
int sock, sock_gui;
int rcv_len;
char buffer[MAX_DATA_FROM_GUI];
char line[MAX_LINE_LENGTH];
int cycle;
uint32_t maxx, maxy;

void execute_line(std::string line) {
  if (strcmp(line.c_str(), "RIGHT_KEY_UP") == 0) {
    if (msg_to_serv.turn_direction == 1) {
      msg_to_serv.turn_direction = 0;
    }
    return;
  }
  if (strcmp(line.c_str(), "LEFT_KEY_UP") == 0) {
    if (msg_to_serv.turn_direction == -1) {
      msg_to_serv.turn_direction = 0;
    }
    return;
  }
  if (strcmp(line.c_str(), "RIGHT_KEY_DOWN") == 0) {
    if (msg_to_serv.turn_direction == 0) {
      msg_to_serv.turn_direction = 1;
    }
    return;
  }
  if (strcmp(line.c_str(), "LEFT_KEY_DOWN") == 0) {
    if (msg_to_serv.turn_direction == 0) {
      msg_to_serv.turn_direction = -1;
    }
    return;
  }
}

void readLinesFromGuiOutput(char *output) {
  uint32_t len = 0;
  uint32_t len_on_line = 0;
  // To make read first line possible.
  char c = output[0];
  std::string line;
  while (c != 0 && len < MAX_DATA_FROM_GUI && len_on_line < MAX_LINE_LENGTH) {
     c = output[len];
     if (c == 0) {
       break;
     }
     if (c == '\n') {
       execute_line(line);
       line = "";
       len_on_line = 0;
     }
     else {
       line += c;
       len_on_line++;
     }
     len++;
  }
}

static void send_message(int sock,
                         struct message_from_client_to_server* msg_to_serv,
                         int player_name_size) {
  // PUSTY PLAYER_NAME = TYLKO OBSERWUJĘ
  struct message_from_client_to_server msg;
  msg.session_id = htobe64(msg_to_serv->session_id);
  msg.turn_direction = msg_to_serv->turn_direction;
  msg.next_expected_event_no = htobe32(msg_to_serv->next_expected_event_no);
  memcpy(msg.player_name, msg_to_serv->player_name, sizeof(char) *player_name_size);

  if (send(sock, &msg, DEFAULT_MESSAGE_TO_SERV_SIZE + player_name_size, 0) != DEFAULT_MESSAGE_TO_SERV_SIZE + player_name_size)
    syserr("send");
}

std::string operate_event_data(char *input, unsigned int it, unsigned int len, unsigned int event_type) {
  unsigned int end = it + len;
  unsigned int name_size;
  uint32_t x, y;
  uint8_t pnr;
  char sign;
  std::string result = "";
  switch (event_type) {
    default:
      // TO ZDARZENIE POMIJAMY! NIE MA FATALA
      // fatal("No event type!");
      break;
    case 0:
      result += "NEW_GAME";
      memcpy(&maxx, input + it, 4);
      maxx = be32toh(maxx);
      result += " " + std::to_string(maxx);
      it += 4;
      memcpy(&maxy, input + it, 4);
      maxy = be32toh(maxy);
      result += " " + std::to_string(maxy);
      it += 4;
      curr_players_nr = 0;
      player_names[curr_players_nr] = "";
      while (it < end) {
        if (curr_players_nr >= MAX_NUMBER_OF_PLAYERS) {
          fatal ("Too many players in event datagram.");
        }
        memcpy(&sign, input + it, 1);
        it++;
        if (sign == 0) {
          name_size = player_names[curr_players_nr].size();
          if (!(name_size >= 1 && name_size <= MAX_PLAYER_NAME_SIZE))
            fatal ("Wrong player name in event datagram (wrong size).");
          result += " " + player_names[curr_players_nr];
          curr_players_nr++;
          player_names[curr_players_nr] = "";
        }
        else if (!(sign >= 33 && sign <= 126))
          fatal ("Wrong player name in event datagram (wrong sign).");
        else {
          player_names[curr_players_nr] += sign;
        }
      }
      break;
    case 1:
      result += "PIXEL";
      memcpy(&pnr, input + it, 1);
      it += 1;
      memcpy(&x, input + it, 4);
      x = be32toh(x);
      it += 4;
      memcpy(&y, input + it, 4);
      y = be32toh(y);
      it += 4;
      if (x >= maxx || y >= maxy) {
        fatal("Wrong pixel coordinates in datagram.");
      }
      result += " " + std::to_string(x) + " " + std::to_string(y) + " " + player_names[pnr];
      break;
    case 2:
      result += "PLAYER_ELIMINATED";
      memcpy(&pnr, input + it, 1);
      it += 1;
      if (pnr > curr_players_nr) {
        fatal("Wrong player number in datagram!");
      }
      result += " " + player_names[pnr];
      break;
    case 3:
      result += "GAME_OVER";
      break;
  }
  return result + "\n";
}

unsigned int operate_event(char *input, unsigned int it, unsigned int len) {
  uint32_t it_beg = it;
  if (len < it + 13) {
    fatal("Wrong size of event datagram!");
  }
  uint32_t event_len, event_no, crc32;
  int8_t event_type;
  event_data = "";
  memcpy(&event_len, input + it, 4);
  event_len = be32toh(event_len);
  it += 4;
  if (len < it + event_len + 4) {
    fatal("Wrong size of event datagram!");
  }
  memcpy(&event_no, input + it, 4);
  event_no = be32toh(event_no);
  it += 4;
  memcpy(&event_type, input + it, 1);
  it += 1;
  event_data = operate_event_data(input, it, event_len - 5, event_type);
  it += (event_len - 5);
  memcpy(&crc32, input + it, 4);
  crc32 = be32toh(crc32);
  it += 4;

  uint32_t crc32_shouldbe = crc(input + it_beg, event_len + 4);
  if (crc32 != crc32_shouldbe) {
    return len;
  }
  // if (event_no < msg_to_serv.next_expected_event_no && event_no != 0) {
  //   fatal("Wrong event number.");
  // }
  msg_to_serv.next_expected_event_no = event_no + 1;
  if (event_type >= 0 && event_type <= 2) {
    if (write(sock_gui, event_data.c_str(), strlen (event_data.c_str())) < 0)
      fatal("writing on stream socket");
  }
  return it;
}


int main(int argc, char *argv[]) {

  auto start = std::chrono::high_resolution_clock::now();
  auto last = start;
  auto elapsed = std::chrono::high_resolution_clock::now() - last;
  long long passed_microseconds = 0;
  unsigned int it_msg;
  uint32_t game_id;
  int rc;

  auto from_beg = start.time_since_epoch();


  msg_to_serv.session_id = std::chrono::duration_cast<std::chrono::microseconds>(from_beg).count();
  msg_to_serv.turn_direction = 1;
  msg_to_serv.next_expected_event_no = 0;
  struct addrinfo addr_hints, addr_hints_gui, *addr_result;
  struct sockaddr_in addr;
  socklen_t addr_size;
  ssize_t len;
  for (unsigned int i = 0; i < MAX_PLAYER_NAME_SIZE; ++i) {
    msg_to_serv.player_name[i] = 0;
  }
  char game_server_host[MAX_SERVER_HOST_NAME + 1];
  for (unsigned int i = 0; i <= MAX_SERVER_HOST_NAME; ++i) {
    game_server_host[i] = 0;
  }
  unsigned int game_server_host_size = 0;
  int game_server_port = DEFAULT_GAME_PORT;
  char gui_server_host[MAX_SERVER_HOST_NAME + 1];
  for (unsigned int i = 0; i <= MAX_SERVER_HOST_NAME; ++i) {
    gui_server_host[i] = 0;
  }
  unsigned int gui_server_host_size = 0;
  int gui_server_port = DEFAULT_USER_PORT;
  bool is_gui_server_port_given = false;
  bool is_host_port_given = false;
  bool finished_sending_to_gui = true;
  unsigned int player_name_size = 0;

  char c, *inbuf;

  if (!(argc >= 3 && argc <= 4))
    fatal("Usage: %s player_name game_server_host[:port] [ui_server_host[:port]]",
          argv[0]);

  while (sscanf(argv[1] + player_name_size,
                "%c",
                &msg_to_serv.player_name[player_name_size]) == 1) {
    if (!(msg_to_serv.player_name[player_name_size] >= 33 &&
          msg_to_serv.player_name[player_name_size] <= 126)) {
      fatal("Invalid sign in player name on %d, %c",
            player_name_size,
            msg_to_serv.player_name[player_name_size]);
    }
    player_name_size++;
    if (player_name_size > MAX_PLAYER_NAME_SIZE)
      break;
  }
  if (player_name_size > MAX_PLAYER_NAME_SIZE) {
    fatal("Player name too long! Max length is %d", MAX_PLAYER_NAME_SIZE);
  }


  while (sscanf(argv[2] + game_server_host_size,
                "%c",
                game_server_host + game_server_host_size) == 1) {
    if (game_server_host[game_server_host_size] == ':') {
      is_host_port_given = true;
      break;
    }
    game_server_host_size++;
    if (game_server_host_size > MAX_SERVER_HOST_NAME)
      break;
  }
  game_server_host[game_server_host_size] = 0;
  if (game_server_host_size > MAX_SERVER_HOST_NAME)
    fatal("Game server host name is too long!");

  if (is_host_port_given &&
      sscanf(argv[2] + game_server_host_size + 1, "%d%c", &game_server_port, &c) != 1) {
    fatal("Invalid game server port %s", argv[2] + game_server_host_size + 1);
  }

  if (!is_port_valid(game_server_port))
    fatal("Invalid game server port %d", game_server_port);

  if (argc == 4) {
    while (sscanf(argv[3] + gui_server_host_size,
                  "%c",
                  gui_server_host + gui_server_host_size) == 1) {
      if (gui_server_host[gui_server_host_size] == ':') {
        is_gui_server_port_given = true;
        break;
      }
      gui_server_host_size++;
      if (gui_server_host_size > MAX_SERVER_HOST_NAME)
        break;
    }
    gui_server_host[gui_server_host_size] = 0;
    if (gui_server_host_size > MAX_SERVER_HOST_NAME)
      fatal("GUI host name is too long!");

    if (is_gui_server_port_given &&
        sscanf(argv[3] + gui_server_host_size + 1, "%d%c", &gui_server_port, &c) != 1) {
      fatal("Invalid gui port %s", argv[3] + gui_server_host_size + 1);
    }
  } else {
    gui_server_host_size = sizeof(DEFAULT_SERVER_NAME);
    memcpy(gui_server_host, DEFAULT_SERVER_NAME, sizeof(DEFAULT_SERVER_NAME));
  }

  if (!is_port_valid(gui_server_port))
    fatal("Invalid gui server port %d", gui_server_port);



  if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
    syserr("signal");

  // DLA SERWERA
  sock = socket(AF_INET, SOCK_DGRAM, 0);
  if (sock < 0)
    syserr("socket - server.");

  /* Trzeba się dowiedzieć o adres internetowy serwera. */
  memset(&addr_hints, 0, sizeof(struct addrinfo));
  addr_hints.ai_family = AF_INET;
  addr_hints.ai_socktype = SOCK_DGRAM;
  addr_hints.ai_protocol = IPPROTO_UDP;
  rc = getaddrinfo(game_server_host,
                       std::to_string(game_server_port).c_str(),
                       &addr_hints,
                       &addr_result);
  if (rc == EAI_SYSTEM)
    syserr("getaddrinfo: %s", gai_strerror(rc));
  else if (rc != 0)
    fatal("getaddrinfo: %s", gai_strerror(rc));

    
  /* Nawiązujemy połączenie. */
  if (connect(sock, addr_result->ai_addr, addr_result->ai_addrlen) != 0)
    syserr("connect");

  freeaddrinfo(addr_result);

  // DLA GUI

  sock_gui = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (sock_gui < 0) {
    syserr("socket - gui.");
  }

  /* Trzeba się dowiedzieć o adres internetowy serwera. */
  memset(&addr_hints_gui, 0, sizeof(struct addrinfo));
  addr_hints_gui.ai_flags = 0;
  addr_hints_gui.ai_family = AF_INET;
  addr_hints_gui.ai_socktype = SOCK_STREAM;
  addr_hints_gui.ai_protocol = IPPROTO_TCP;
  /* I tak wyzerowane, ale warto poznać pola. */
  addr_hints_gui.ai_addrlen = 0;
  addr_hints_gui.ai_addr = NULL;
  addr_hints_gui.ai_canonname = NULL;
  addr_hints_gui.ai_next = NULL;
  rc =  getaddrinfo(gui_server_host,
                    std::to_string(gui_server_port).c_str(),
                    &addr_hints_gui,
                    &addr_result);
  if (rc != 0) {
    fprintf(stderr, "rc=%d\n", rc);
    syserr("getaddrinfo: %s", gai_strerror(rc));
  }

  if (connect(sock_gui, addr_result->ai_addr, addr_result->ai_addrlen) != 0) {
    syserr("connect");
  }
  freeaddrinfo(addr_result);


  /* Niebokujące sockety :). */
  fcntl(sock, F_SETFL, O_NONBLOCK);
  fcntl(sock_gui, F_SETFL, O_NONBLOCK);

  inbuf = (char *) malloc(MAX_UDP_FROM_SERVER_TO_CLIENT);
  if (!inbuf)
    syserr("malloc");

  for (;;) {
    elapsed = std::chrono::high_resolution_clock::now() - start;
    passed_microseconds = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
    while ((cycle + 1) * MICROSECONDS_PAUSE < passed_microseconds) {
      cycle++;
    }
    if (abs(cycle * MICROSECONDS_PAUSE - passed_microseconds) < TIME_EPS || cycle == 0) {
      cycle++;
      send_message(sock, &msg_to_serv, player_name_size);
    }
    addr_size = sizeof(addr);
    len = recvfrom(sock,
                   inbuf,
                   MAX_UDP_FROM_SERVER_TO_CLIENT,
                   0,
                   (struct sockaddr *)&addr,
                   &addr_size);
    if (len > 0) {
      memcpy(&game_id, inbuf, 4);
      game_id = be32toh(game_id);
      finished_sending_to_gui = false;
      it_msg = 4;
      while (!finished_sending_to_gui) {
          if (it_msg == len) {
            finished_sending_to_gui = true;
            break;
          }
          it_msg = operate_event(inbuf, it_msg, len);
      }
    }

    memset(buffer, 0, sizeof(buffer));
    rcv_len = read(sock_gui, buffer, sizeof(buffer) - 1);
    if (rcv_len < 0) {
      continue;
    }

    readLinesFromGuiOutput(buffer);
  }
}
