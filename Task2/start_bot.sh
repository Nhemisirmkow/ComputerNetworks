#!/bin/bash
DEFAULT_GUI_PORT=12389
DEFAULT_SERVER_PORT=$2
GAME_HOST="10.1.0.3"
GUI_HOST="localhost"
NR_OF_BOT=$[$1]

ssh -T -X studia "cd ./SIK/zadania/; ./siktacka-client1 BOT_NR_"$NR_OF_BOT" "$GAME_HOST":"$DEFAULT_SERVER_PORT" "$GUI_HOST":"$[$DEFAULT_GUI_PORT + 2 * $NR_OF_BOT]""
