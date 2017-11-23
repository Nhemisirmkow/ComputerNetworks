#!/bin/bash
DEFAULT_GUI_PORT=12389
DEFAULT_SERVER_PORT=40406
GAME_HOST="10.1.0.3"
GUI_HOST="localhost"
NR_OF_GUI=$1

./start_normal_gui.sh $NR_OF_GUI &
sleep 2
ssh -T -X studia "cd ./SIK/zadania/; ./siktacka-client Mistrzuu "$GAME_HOST":"$DEFAULT_SERVER_PORT" "$GUI_HOST":"$[$DEFAULT_GUI_PORT + 2 * $NR_OF_GUI]""

for i in 0 ; do
  ssh studia "pkill -o -u mm371177; pkill -o -u mm371177; pkill -o -u mm371177"
done


# ssh -T -X studia "cd ./SIK/zadania/; ./siktacka-client1 BOT_NR_"$NR_OF_BOT" "$GAME_HOST":"$DEFAULT_SERVER_PORT" "$GUI_HOST":"$[$DEFAULT_GUI_PORT + 2 * $NR_OF_BOT]""
