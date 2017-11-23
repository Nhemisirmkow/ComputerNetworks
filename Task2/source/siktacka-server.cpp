#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <list>
#include <string>
#include <memory>
#include <chrono>
#include <limits.h>
#include <poll.h>
#include <signal.h>
#include <algorithm>
#include <sys/socket.h>
#include <netdb.h>
#include <fcntl.h>

#include "consts.h"
#include "err.h"
#include "common.h"
#include "randomInt.cpp"
#include "player.cpp"
#include "gameState.cpp"
#include "event.cpp"
#include "clientClass.cpp"


int main(int argc, char** argv) {
  /* Wczytywanie parametrów */
  int c;
  char *end_ptr;
  int width = 800, height = 600, port = 12345, rounds_per_sec = 50, turning_speed = 6;
  uint64_t random_seed = time(NULL);
  bool check_digits;
  bool non_option_arguments = false;
  while ((c = getopt(argc, argv, "W:H:p:s:t:r:")) != -1) {
    check_digits = true;
    switch(c) {
      case 'W':
        width = atoi(optarg);
        if (!is_height_or_width_valid(width))
          fatal("Wrong width.");
        break;
      case 'H':
        height = atoi(optarg);
        if (!is_height_or_width_valid(height))
          fatal("Wrong height.");
        break;
      case 'p':
        port = atoi(optarg);
        if (!is_port_valid(port))
          fatal("Wrong port.");
        break;
      case 's':
        rounds_per_sec = atoi(optarg);
        if (!is_rounds_per_sec_valid(rounds_per_sec))
          fatal("Wrong rounds_per_sec parameter.");
        break;
      case 't':
        turning_speed = atoi(optarg);
        if(!is_turning_speed_valid(turning_speed))
          fatal("Wrong turning speed.");
        break;
      case 'r':
        random_seed = strtoul(optarg, &end_ptr, 10);
        if(!(strcmp(optarg, "0") == 0) && !is_random_seed_valid(random_seed))
          fatal("Wrong random_seed parameter.");
        break;
      case '?':
        if (optopt == 'c')
          fprintf (stderr, "Option -%c requires an argument.\n", optopt);
        else if (isprint (optopt))
          fprintf (stderr, "Unknown option `-%c'.\n", optopt);
        else
          fprintf (stderr,
                   "Unknown option character `\\x%x'.\n",
                   optopt);
        return 1;
      default:
        abort();
    }
    if (check_digits && !is_digits(optarg)) {
      fatal("Argument %c does not contain only digits.", c);
    }
  }
  for (auto index = optind; index < argc; index++) {
    printf ("Non-option argument %s\n", argv[index]);
    non_option_arguments = true;
  }
  if (non_option_arguments) {
    fatal("Non-option arguments appeared.");
  }

  setRandomInt(random_seed);
  /* Koniec wczytywania parametrów */

  EventShared events[MAX_NUMBER_OF_EVENTS];
  uint32_t event_no = 0;
  SharedPlayer players[MAX_NUMBER_OF_PLAYERS];
  Client clients[MAX_NUMBER_OF_CLIENTS];
  uint32_t no_of_active_players = 0;
  uint32_t no_of_clients = 0;
  GameState g = GameState(events,
                          event_no,
                          width,
                          height,
                          players,
                          no_of_active_players);
  struct sockaddr_in6 addr, server_address;
  socklen_t addr_size;
  ssize_t len;
  char *inbuf;
  int sock, client_flag;


  sock = socket(AF_INET6, SOCK_DGRAM, 0);
  if (sock < 0)
    syserr("socket.");
  server_address.sin6_family = AF_INET6;
  server_address.sin6_addr = in6addr_any;
  server_address.sin6_port = htons(port);

  // bind the socket to a concrete address
  if (bind(sock, (struct sockaddr *) &server_address,
      (socklen_t) sizeof(server_address)) < 0)
    syserr("bind");

  fcntl(sock, F_SETFL, O_NONBLOCK);


  inbuf = (char *) malloc(MAX_UDP_FROM_SERVER_TO_CLIENT);
  if (!inbuf)
    syserr("malloc");

  bool client_found;
  uint32_t client_no;
  uint32_t old_event_no;
  for (;;) {
    kickInactivePlayers(clients, no_of_clients);
    old_event_no = event_no;
    g.makeTurnOrStartGame(events, event_no, rounds_per_sec, turning_speed);
    if (event_no > old_event_no) {
      for (unsigned int o = 0; o < no_of_clients; ++o) {
        clients[o].sendEventsFromNumber(events, event_no, sock, old_event_no, g.game_id);
      }
    }
    else if (event_no < old_event_no && old_event_no != 0) {
      for (unsigned int o = 0; o < no_of_clients; ++o) {
        clients[o].sendEventsFromNumber(events,
                                        g.event_no_last,
                                        sock,
                                        old_event_no,
                                        g.last_game_id);
        clients[o].sendEventsFromNumber(events, event_no, sock, 0, g.game_id);
      }
    }
    addr_size = sizeof(addr);
    len = recvfrom(sock,
                   inbuf,
                   MAX_UDP_FROM_SERVER_TO_CLIENT,
                   0,
                   (struct sockaddr *)&addr,
                   &addr_size);
    if (len > 0) {
      if (!check(inbuf, len)) {
        break;
      }
      client_found = false;
      client_no = -1;
      client_flag = -1;
      for (unsigned int i = 0; i < no_of_clients; ++i) {
        client_flag = clients[i].checkAndOperateClient(&addr, inbuf, len);
        if (client_flag != 0) {
          client_found = true;
          client_no = i;
        }
      }
      if (!client_found && no_of_clients < MAX_NUMBER_OF_CLIENTS) {
        clients[no_of_clients] = Client(&addr, inbuf, len);
        clients[no_of_clients].sendAllEvents(events, event_no, sock, g.game_id);
        g.addPlayer(clients[no_of_clients].p);
        no_of_clients++;
      }
      else if (client_found){
        // if 3 - name is taken = ignore
        if (client_flag != 3)
          clients[client_no].sendEventsFromNumber(events,
                                                  event_no,
                                                  sock,
                                                  clients[client_no].next_expected_event_no,
                                                  g.game_id);
      }
    }
  }
  return 0;
}
