/* Stałe na potrzeby zadania z Sieci komputrowych. */
/* 7.06.2017 Marcin Michorzewski*/

#ifndef CONSTS_H
#define CONSTS_H

/* used in :
*   gameState.cpp
*/
const unsigned int MAX_NUMBER_OF_PLAYERS = 42;
const unsigned int MAX_NUMBER_OF_CLIENTS = 42;
const unsigned int MAX_NUMBER_OF_EVENTS = 4000;
const unsigned int MAX_PLAYER_NAME_SIZE = 64;
const unsigned int MICROSECONDS_IN_SECOND = 1000000;
const bool DEBUG = true;
const long double PI = 3.14159265;
/* used in rand.cpp */
const unsigned long multiply = 279470273;
const unsigned long modulo = 4294967291;
/* helper */
const unsigned int MAX_BOARD_SIZE = 2500;
/* used in gameState.cpp */
const unsigned int MAX_BOARD_SIZE_Y = MAX_BOARD_SIZE;
const unsigned int MAX_BOARD_SIZE_X = MAX_BOARD_SIZE;
const int HEIGHT_OR_WIDTH_MAX = 4000;

const unsigned int MAX_SERVER_HOST_NAME = 300;
// In microseconds
const unsigned long TIME_EPS = 10000;
const unsigned long TIME_EPS2 = 10000;
const unsigned int DEFAULT_MESSAGE_TO_SERV_SIZE = 13;
const unsigned int MICROSECONDS_PAUSE = 20000;

const unsigned int MAX_UDP_FROM_SERVER_TO_CLIENT = 512;
const unsigned int MAX_EVENT_DATA = 512;
const unsigned int READ_TIMEOUT = 100;
const unsigned int MAX_DATA_FROM_GUI = 512;
const unsigned int MAX_LINE_LENGTH = 200;
const int TURNING_SPEED_MAX = 360;
const int ROUNDS_PER_SEC_MAX = 500;
#endif /* CONSTS_H */
