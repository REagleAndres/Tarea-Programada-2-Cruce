# Compilador
CC = gcc

# Flags de compilación
CFLAGS = -Wall -Wextra -O2 -g

# Carpetas
SRC_DIR = src
INC_DIR = include
BIN_DIR = bin

# Nombre del ejecutable
TARGET = $(BIN_DIR)/programa

# Archivos fuente
SOURCES = \
	$(SRC_DIR)/cruce.c


SRC := $(filter-out src/main.c, $(wildcard src/*.c))

# Archivos objeto
OBJECTS = $(SOURCES:.c=.o)

# Regla principal
all: $(TARGET)

# Crear ejecutable
$(TARGET): $(OBJECTS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS)

# Compilar archivos .c -> .o
$(SRC_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -I$(INC_DIR) -c $< -o $@

# Ejecutar programa por default en fase 1
run: all
	./$(TARGET) fase_1

run_fase_2: all
	./$(TARGET) fase_2

.PHONY: test

test:
	gcc -g tests/test.c src/busqueda_lineal.c src/busqueda_MPI.c src/busqueda_pthread.c src/cadenas.c src/parametros.c -Iinclude -o test && ./test

# Ejecutar con MPI
run_mpi: all
	mpirun -np 4 ./$(TARGET)

run_valgrind: all
	valgrind --leak-check=full --show-leak-kinds=all ./$(TARGET)


# Limpiar archivos compilados
clean:
	rm -f $(SRC_DIR)/*.o
	rm -rf $(BIN_DIR)

# Evitar conflictos con nombres de archivos
.PHONY: all run run_mpi clean