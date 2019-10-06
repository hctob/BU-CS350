# build an executable named myprog from myprog.c
prog2tree: prog2tree.c parser.c parser.h
	gcc -g -Wall -o prog2tree prog2tree.c parser.c

simplefork: simplefork.c
	gcc -g -Wall -o simplefork simplefork.c

clean: 
	$(RM) prog2tree
clean_fork:
	$(RM) simplefork

