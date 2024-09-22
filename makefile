CC = g++
FLAGS = -Wall
LIBS =	-lmingw32 -lSDL2main -lSDL2 

sim.exe : sim.cpp
	$(CC) sim.cpp -c -o sim.o $(FLAGS)
	$(CC) sim.o -o sim.exe $(FLAGS) $(LIBS)
