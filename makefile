# Compilador
CC = mpicc

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
	$(SRC_DIR)/main.c \
	$(SRC_DIR)/cadenas.c \
	$(SRC_DIR)/busqueda_lineal.c \
	$(SRC_DIR)/busqueda_MPI.c \
	$(SRC_DIR)/busqueda_pthread.c \
	$(SRC_DIR)/parametros.c

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

# Ejecutar programa
run: all
	./$(TARGET)

.PHONY: test

test:
	mpicc -g tests/test.c src/busqueda_lineal.c src/busqueda_MPI.c src/busqueda_pthread.c src/cadenas.c src/parametros.c -Iinclude -o test && ./test

# Ejecutar con MPI
run_mpi: all
	mpirun -np 4 ./$(TARGET)

run_valgrind: all
	mpirun -np 4 valgrind --gen-suppressions=all ./$(TARGET)

# Limpiar archivos compilados
clean:
	rm -f $(SRC_DIR)/*.o
	rm -rf $(BIN_DIR)

# Evitar conflictos con nombres de archivos
.PHONY: all run run_mpi clean