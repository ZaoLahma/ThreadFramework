CCOMMAND = g++
CFLAGS = -Wall -c -Wextra --std=c++11 
LINKARGS = -lpthread
SOURCES = $(wildcard src/*.cpp)
SOURCES += $(wildcard src/internal/*.cpp)
TEST_SOURCES += $(wildcard test/*.cpp)
OBJECTS = $(SOURCES:.cpp=.o)
TEST_OBJECTS = $(TEST_SOURCES:.cpp=.o)
INC_DIRS = -I./inc
EXE_NAME = thread_framework


$(EXE_NAME): $(OBJECTS) $(TEST_OBJECTS)
	$(CCOMMAND) $(OBJECTS) $(TEST_OBJECTS) $(LINKARGS) -o $(EXE_NAME)

%.o: %.cpp
	$(CCOMMAND) $(INC_DIRS) -c $(CFLAGS) $< -o $@

lib: $(OBJECTS)
	ar rvs jobdispatcher.a $(OBJECTS)

clean:
	rm -f $(EXE_NAME) $(OBJECTS) $(TEST_OBJECTS)
	
