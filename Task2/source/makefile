CORE = siktacka-server.cpp
CORE2 = client.cpp
TARGET = siktacka-server
TARGET2 = siktacka-client
FLAGS = -std=c++14 -Wall -O2

all:
	g++ $(FLAGS) -o $(TARGET2) -g $(CORE2)
	g++ $(FLAGS) -o $(TARGET) -g $(CORE)

.PHONY: clean
clean:
	rm siktacka-server siktacka-client
