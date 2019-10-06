all: lfs-simulator

lfs-simulator: main.o Drive.o IMap.o Segment.o Block.o macros.o
	c++ $^ -o $@.bin

%.o : %.cpp
	c++ -I . -c $< -o $@

clean:
	rm -f *.o *.bin
