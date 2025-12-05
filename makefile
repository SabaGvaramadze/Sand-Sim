CC = g++
FLAGS = -Wall -g
LIBS = -lSDL2main -lSDL2 

sim.exe : sim.cpp
	$(CC) sim.cpp -c -o sim.o $(FLAGS)
	$(CC) sim.o -o sim $(FLAGS) $(LIBS)
