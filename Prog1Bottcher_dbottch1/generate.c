#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include "parse.h"

struct global_t g;
#define atoa(x) #x

int main(int argc, char **argv, char *envp[]) {
	FILE *op;
	c_parser(argc, argv);
	int i;
	if(g.oFlag == 0) {
		for(i = 0; i < g.nFlag; i += 1) {
		//srand((unsigned) time(&t)); //new random seed for each integer generated
			int num = g.minFlag + rand() / (RAND_MAX / (g.maxFlag - g.minFlag + 1) + 1);	
       			printf("%d\n", num);
		}
	}
	else {
		op = fopen(g.op, "w");
		for(i = 0; i < g.nFlag; i += 1) {
			int num = g.minFlag + rand() / (RAND_MAX / (g.maxFlag - g.minFlag + 1) + 1);
			char* str;
			asprintf (&str, "%i\n", num);
			fprintf(op, str);
			free(str);
		}
		fclose(op);
		
	}
	return 0;
}
