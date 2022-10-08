CC := gcc
std=c99 -pedantic -Wall -Wextra -D_POSIX_C_SOURCE=200112L

SRC := src
INC := include
BUILD := build
BIN := bin
MAIN_EXECUTABLE := lisb
TEST_EXECUTABLE := test

SRCEXT := c
SOURCES := $(shell find $(SRC) -type f -name *.$(SRCEXT))
MAIN_SOURCES := $(patsubst $(SRC)/%test.$(SRCEXT),, $(SOURCES))
MAIN_OBJECTS := $(patsubst $(SRC)/%,$(BUILD)/%,$(MAIN_SOURCES:.$(SRCEXT)=.o))
TEST_SOURCES := $(patsubst $(SRC)/%main.$(SRCEXT),, $(SOURCES))
TEST_OBJECTS := $(patsubst $(SRC)/%,$(BUILD)/%,$(TEST_SOURCES:.$(SRCEXT)=.o))

MAIN_LIBRARIES := -lreadline -lncurses
TEST_LIBRARIES := $(MAIN_LIBRARIES) -lcunit
INCLUDES := -I $(SRC) -I $(INC)

all: main

debug: CC_FLAGS += -DDEBUG_TRACE_EXECUTION -DDEBUG_PRINT_CODE -g
debug: main

main: $(BIN)/$(MAIN_EXECUTABLE)

$(BIN)/$(MAIN_EXECUTABLE): $(MAIN_OBJECTS)
	$(CC) $^ -o $(BIN)/$(MAIN_EXECUTABLE) $(MAIN_LIBRARIES)

test: $(BIN)/$(TEST_EXECUTABLE)

$(BIN)/$(TEST_EXECUTABLE): $(TEST_OBJECTS)
	$(CC) $^ -o $(BIN)/$(TEST_EXECUTABLE) $(TEST_LIBRARIES)

$(BUILD)/%.o: $(SRC)/%.$(SRCEXT)
	@mkdir -p $(@D)
	$(CC) $(CC_FLAGS) $(INCLUDES) -c -o $@ $<

clean:
	$(RM) -r $(BUILD)/*
	$(RM) -r $(BIN)/*

.PHONY: clean
