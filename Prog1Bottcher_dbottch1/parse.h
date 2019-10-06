#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

struct global_t {
	char uFlag; //uFlag, prints usage string
	char* usageString;
	unsigned int nFlag; //nFlag, number of integers to be generated or sorted by the programs; 100 default
	unsigned int minFlag; //minFlag, >= 1; 1 default
	unsigned int maxFlag; //maxFlag, > min; 255 default
	char iFlag; //iFlag, input file specified by <input_file>. not available in generator
	char oFlag; //oFlag, output file specified by <output_file>. stdout otherwise
	unsigned int sFlag; //sFlag, seed for generator function. not available in sorter.
	char cFlag; //cFlag, specifies file to write count information. stdout otherwise

	char* op; //output file name
	char* ip; //input file name
	char* cp; //count file name
};

extern struct global_t g;

void c_parser(int argc, char **argv);
