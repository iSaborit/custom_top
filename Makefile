# Compilador y Flags
CC       := gcc
CFLAGS   := -Wall -Wextra -Iinclude
LDFLAGS  := -lncurses

# Directorios
SRC_DIR  := src
OBJ_DIR  := obj
BIN_DIR  := bin

# Nombre del ejecutable final
TARGET   := $(BIN_DIR)/mi_programa

# Archivos (Busca todos los .c en src/)
SRCS     := $(wildcard $(SRC_DIR)/*.c)
OBJS     := $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

# Regla por defecto
all: $(TARGET)

# Enlace del ejecutable
$(TARGET): $(OBJS) | $(BIN_DIR)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

# Compilación de objetos (.c -> .o)
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Crear directorios si no existen
$(BIN_DIR) $(OBJ_DIR):
	mkdir -p $@

# Limpieza
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

.PHONY: all clean
