CC := g++-7
CFLAGS := -O3
STD := -std=c++14

all: heap.cpp
	$(CC) $(STD) $(CFLAGS) heap.cpp -o heap 
