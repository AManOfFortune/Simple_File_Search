#Makefile für main.cpp
all: prog

prog: main.cpp
	g++ -std=c++17 -Wall -o prog main.cpp
	
clean:
	rm -f prog