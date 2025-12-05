CC = g++
FLAGS = -Wall -g -std=c++11
LIBS = -lSDL2main -lSDL2 -lpthread

sim.exe : sim.cpp
	$(CC) sim.cpp -c -o sim.o $(FLAGS)
	$(CC) sim.o -o sim $(FLAGS) $(LIBS)
