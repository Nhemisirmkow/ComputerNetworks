#!/bin/bash
# DEFAULT_GUI_PORT=12389
TIME=$1
DEFAULT_SERVER_PORT=42489
# GAME_HOST="10.1.0.3"
# GUI_HOST="localhost"
# NR_OF_BOT=$[$1]

for i in 0 1 2 3 4 5 6; do
  ssh studia "pkill -o -u mm371177; pkill -o -u mm371177; pkill -o -u mm371177"
done

for i in 0 1 2 3 4; do
  ./start_gui.sh $i &
  sleep 2
  ./start_bot.sh $i $DEFAULT_SERVER_PORT &
  sleep 1
done

./start_gui.sh 5 &
sleep 2
./start_bot.sh 5 $DEFAULT_SERVER_PORT &
sleep $TIME


for i in 0 1 2 3 4 5 6 ; do
  ssh studia "pkill -o -u mm371177; pkill -o -u mm371177; pkill -o -u mm371177"
done

# ssh -T -X studia "cd ./SIK/zadania/; ./siktacka-client1 BOT_NR_"$NR_OF_BOT" "$GAME_HOST":"$DEFAULT_SERVER_PORT" "$GUI_HOST":"$[$DEFAULT_GUI_PORT + 2 * $NR_OF_BOT]""
