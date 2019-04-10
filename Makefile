CC        := gcc
TARGET    := example.a
FLAGS     := -std=c99 -Wall -Wextra -Werror -O3
BUILD     := ./build
OBJ_DIR   := $(BUILD)/objects

SRC       :=          \
	$(wildcard src/*.c)

COBJ  := $(SRC:%.c=$(OBJ_DIR)/%.o)

$(OBJ_DIR)/%.o: %.c
	@mkdir -p $(@D)
	$(CC) $< $(FLAGS) -c -o $@

$(BUILD)/$(TARGET): $(COBJ)
	$(CC) $^ $(FLAGS) -o $@

.PHONY: clean
clean:
	-@rm -rvf $(BUILD)/*