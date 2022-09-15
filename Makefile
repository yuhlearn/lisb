CC := gcc
CC_FLAGS := -Wall -Wextra 

SRC := src
INC := include
BUILD := build
BIN := bin
MAIN_EXECUTABLE := main
TEST_EXECUTABLE := test

SRCEXT := c
SOURCES := $(shell find $(SRC) -type f -name *.$(SRCEXT))
MAIN_SOURCES := $(patsubst $(SRC)/%test.$(SRCEXT),, $(SOURCES))
MAIN_OBJECTS := $(patsubst $(SRC)/%,$(BUILD)/%,$(MAIN_SOURCES:.$(SRCEXT)=.o))
TEST_SOURCES := $(patsubst $(SRC)/%main.$(SRCEXT),, $(SOURCES))
TEST_OBJECTS := $(patsubst $(SRC)/%,$(BUILD)/%,$(TEST_SOURCES:.$(SRCEXT)=.o))

MAIN_LIBRARIES := 
TEST_LIBRARIES := -lcunit
INCLUDES := -I $(SRC) -I $(INC)

all: main

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
	$(RM) -r $(BUILD)
	$(RM) -r $(BIN)/*

.PHONY: clean
