CC = gcc

CFLAGS = `sdl-config --cflags` -O0 -g
# includes and libs
LIBS = `sdl-config --libs` -lSDL_ttf -lSDL_image -lSDL_mixer -lm 
BIN_DIR = living-worlds
PRODUCT = $(BIN_DIR)/living-worlds-x86
SRC_DIR = src
OBJ_DIR = $(SRC_DIR)/objs-x86
SOURCES = $(wildcard $(SRC_DIR)/*.c)
OBJECTS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SOURCES))

all: $(PRODUCT)

$(PRODUCT): $(OBJECTS)
	$(CC) $^ $(INCS) $(CFLAGS) $(LIBS)  -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

clean:
	rm -rf $(OBJ_DIR) $(PRODUCT)