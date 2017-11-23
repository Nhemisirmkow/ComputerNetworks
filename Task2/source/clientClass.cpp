typedef typename std::shared_ptr<struct sockaddr_in6> addressPtr;

class Client{
  public:
    addressPtr addressptr;
    SharedPlayer p;
    std::chrono::time_point<std::chrono::high_resolution_clock> lastTime = std::chrono::high_resolution_clock::now();
    uint64_t id;
    uint32_t next_expected_event_no;
    Client() {}
    Client(struct sockaddr_in6 *_addressPtr, char *input, uint32_t len) {
      lastTime = std::chrono::high_resolution_clock::now();
      addressptr = std::make_shared<struct sockaddr_in6>(sockaddr_in6(*_addressPtr));
      parseAndExecuteMessage(input, len);
    }

    void updateInfo(char *input, uint32_t len) {
      p->setTurnDirection((int8_t) *(input + 8));
      memcpy(&next_expected_event_no, input + 9, 4);
      next_expected_event_no = be32toh(next_expected_event_no);
    }

    void parseAndExecuteMessage(char *input, uint32_t len) {
      uint32_t it = 0;
      memcpy(&id, input + it, 8);
      it += 8;
      id = be32toh(id);
      int8_t turn_direction;
      memcpy(&turn_direction, input + it, 1);
      it += 1;
      memcpy(&next_expected_event_no, input + it, 4);
      it += 4;
      next_expected_event_no = be32toh(next_expected_event_no);
      p = std::make_shared<Player>(Player(input + it, len - 13));
      p->setTurnDirection(turn_direction);
    }
    // Returns 0 - if not the same,
    //         1 - if everything's same or session_id bigger,
    //         2 - if-session_id is lower
    //         3 - if name is taken
    //
    int checkAndOperateClient(struct sockaddr_in6 *_addressPtr, char *input, uint32_t len) {
      if (_addressPtr->sin6_port == addressptr->sin6_port) {
        lastTime = std::chrono::high_resolution_clock::now();
        uint64_t tmp_id;
        memcpy(&tmp_id, input, 8);
        tmp_id = be32toh(tmp_id);
        if (tmp_id < id) {
          return 2;
        }
        id = tmp_id;
        updateInfo(input, len);
        return 1;
      }
      if (len - 13 == this->p->name_size() &&
          strncmp(input + 13, this->p->name, len - 13) == 0) {
        return 3;
      }
      return 0;
    }

    void sendEventsFromNumber(EventShared *events, uint32_t event_no, int sock, uint32_t number, uint32_t game_id) {
      char msg[MAX_UDP_FROM_SERVER_TO_CLIENT];
      game_id = htobe32(game_id);
      memcpy(msg, &game_id, 4);
      uint32_t msg_len = 4;
      uint32_t event_len = 0;
      socklen_t dest_len;
      for (unsigned int i = number; i < event_no; ++i) {
        event_len = events[i]->give_full_length();
          if (event_len + msg_len < MAX_UDP_FROM_SERVER_TO_CLIENT) {
            memcpy(msg + msg_len, events[i]->event_message(), event_len);
            msg_len += event_len;
          }
          else {
            dest_len = sizeof(*addressptr);
            if (sendto(sock, msg, msg_len, 0, (struct sockaddr*) addressptr.get(), dest_len) != msg_len) {
              perror("send");
            }
            memcpy(msg, &game_id, 4);
            msg_len = 4;
          }
      }
      if (msg_len != 0) {
        dest_len = sizeof(*addressptr);
        if (sendto(sock, msg, msg_len, 0, (struct sockaddr*) addressptr.get(), dest_len) != msg_len) {
          perror("send");
        }
        msg_len = 0;
      }
    }

    void sendAllEvents(EventShared *events, uint32_t event_no, int sock, uint32_t game_id) {
      sendEventsFromNumber(events, event_no, sock, 0, game_id);
    }
  private:
};

void kickClient(Client *clients, uint32_t &no_of_clients, uint32_t it) {
  std::swap(clients[no_of_clients - 1], clients[it]);
  clients[no_of_clients - 1].p = std::make_shared<Player>();
  no_of_clients--;
}

void kickInactivePlayers(Client *clients, uint32_t &no_of_clients) {
  auto elapsed = std::chrono::high_resolution_clock::now() - clients[0].lastTime;
  uint64_t microseconds;
  for (unsigned int i = 0; i < no_of_clients; ++i) {
    elapsed = std::chrono::high_resolution_clock::now() - clients[i].lastTime;
    microseconds = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
    if (microseconds > MICROSECONDS_IN_SECOND * 2) {
      kickClient(clients, no_of_clients, i);
    }
  }
}

bool check(char *inbuf, uint32_t len) {
  if (len < 13) return false;
  else {
    int8_t turn_direction;
    memcpy(&turn_direction, inbuf + 8, 1);
    if (!(turn_direction >= -1 && turn_direction <= 1)) {
      return false;
    }
    for (uint32_t i = 13; i < len; ++i) {
      if (!(inbuf[i] >= 33 && inbuf[i] <= 126)) {
        return false;
      }
    }
  }
  return true;
}
