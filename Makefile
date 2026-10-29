CC = g++
CFLAGS = -std=c++23 -O2 -Wall -Wextra 
TARGET = bench
DCOUNT = test
SRC = benchmark.cpp dijkstra_Binary.cpp dijkstra_Pairing.cpp

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

$(DCOUNT): $(SRC)
	$(CC) $(CFLAGS) -DCOUNT_OPS -o $(DCOUNT) $(SRC)

clean: 
	rm  -f $(TARGET)
