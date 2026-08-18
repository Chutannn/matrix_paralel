CC = gcc
CFLAGS = -O3 -fopenmp -Wall
TARGET = matrix_omp.exe
SRC = src/matrix_omp.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET) benchmark_results.csv benchmark_results.json
