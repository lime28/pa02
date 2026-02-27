CXX_FLAGS = -std=c++20 -Ofast

all: runMovies

runMovies: utilities.o main.cpp
	g++ $(CXX_FLAGS) -o runMovies utilities.o main.cpp

utilities.o: utilities.h utilities.cpp
	g++ -c $(CXX_FLAGS) utilities.cpp

clean:
	rm -f *.o
	rm -f runMovies
