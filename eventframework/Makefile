CCOMMAND = g++
CFLAGS = -Wall -Wextra --std=c++11 -lpthread
LINKARGS = 
SRC_FILES = ./src/*.cpp ./test/*.cpp
INC_DIRS = ./inc
EXE_NAME = event_framework

all:
	$(CCOMMAND) $(CFLAGS) -I$(INC_DIRS) $(SRC_FILES) -o $(EXE_NAME) $(LINKARGS)
	
clean:
	rm -rf ./*.o
	rm ./$(EXE_NAME)