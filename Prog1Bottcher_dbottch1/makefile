# build an executable named myprog from myprog.c
prog1sorter: sort.c parse.c parse.h
	gcc -g -Wall -o prog1sorter sort.c parse.c
prog1generator: generate.c parse.c parse.h
	gcc -g -Wall -o prog1generator generate.c parse.c

clean_sort: 
	$(RM) prog1sorter
clean_gen:
	$(RM) prog1generator
