#!/bin/bash
DEFAULT_PORT=12389
NR_OF_GUI=$[2*$1]

ssh -T -X studia "cd ./SIK/zadania/gui/gui3/; ./gui3 "$[$NR_OF_GUI + $DEFAULT_PORT]""
