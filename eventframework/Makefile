CCOMMAND = g++
CFLAGS = -Wall -c -Wextra --std=c++11 
LINKARGS = -lpthread
SRC_FILES = ./src/*.cpp
TEST_FILES = ./test/*.cpp
INC_DIRS = ./inc
EXE_NAME = event_framework

all: test link

test: compile
	$(CCOMMAND) $(CFLAGS) -I$(INC_DIRS) $(TEST_FILES)

compile:
	$(CCOMMAND) $(CFLAGS) -I$(INC_DIRS) $(SRC_FILES)
	
link:
	$(CCOMMAND) -o $(EXE_NAME) ./*.o $(LINKARGS)
	
lib: compile
	ar rvs jobdispatcher.a ./*.o
	
clean:
	rm -rf ./*.o
	rm ./$(EXE_NAME)