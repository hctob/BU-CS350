all: main.o
	g++ -g -Wall main.cpp -o readwrite -pthread
main.o:
	g++ -g -Wall -c main.cpp
clean:
	rm -f *.o *.txt readwrite
