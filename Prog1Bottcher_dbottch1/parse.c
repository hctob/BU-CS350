#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "parse.h"

struct global_t g;

void c_parser(int argc, char **argv) {
	g.uFlag = 0;
	g.usageString = "Usage:\n-u: print usage string\n-n <num>: number of integers to be generated/sorted\n-m <min>: min number\n-M <max>: max number\n-i <input_file>\n-o <output_file>\n-s <seed>: seed for random generator\n-c <count_file>\n";
	g.nFlag = 100;
	g.minFlag = 1;
	g.maxFlag = 255;
	g.iFlag = 0;
	g.sFlag = 0;
	g.oFlag = 0;
	g.cFlag = 0;
	int opt; //opt for getopt()
	while((opt = getopt(argc, argv, "un:m:M:i:s:o:c:")) != -1) {
		switch(opt) {
			case 'u':
				g.uFlag = 1;
				perror(g.usageString);
				exit(1);
				break;
			case 'n':
				g.nFlag = atoi(optarg);
				break;
			case 'm':
				g.minFlag = atoi(optarg);
				if(g.minFlag < 1) { perror("-m cannot be less than 1\n"); exit(1);}
				else {
				}
				break;
			case 'M':
				g.maxFlag = atoi(optarg);
				if(g.maxFlag <= g.minFlag) { perror("-M cannot be less than or equal to min\n"); exit(1);}
				else if( g.maxFlag > 1000000) {
					perror("-M cannot be greater than 1000000\n");
					exit(0);
				}
				break;
			case 'i':
				g.iFlag = 1;
				g.ip = optarg;
				break;
			case 's':
				g.sFlag = atoi(optarg);
				break;
			case 'o':
				g.oFlag = 1;
				g.op = optarg;
				break;
			case 'c':
				g.cFlag = 1;
				g.cp = optarg;
				break;
			default:
				perror("Unrecognized flag. Exiting\n");
				exit(0);
				break;
		}
	}
}
