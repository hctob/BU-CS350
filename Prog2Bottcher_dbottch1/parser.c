#include <sys/types.h>
#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <stdlib.h>
#include "parser.h"

struct flag_data_t flags;

void c_parser(int argc, char **argv) {
	flags.uFlag = 0;
	flags.usageString = "prog2tree [-u] [-N <num_levels>] [-M <num_children>] [-p] [-s <sleep_time>]\n";
	flags.nFlag = 1; //1 <= N <= 4 (1 = parent, 2 = child, 3 = grandchild, 4 = more)
	flags.mFlag = 1; //1 <= M <= 3
	flags.pFlag = 0; //leaf processes call pause() upon realizing that they are leaf processes
	flags.sFlag = 0; //if neither -p nor -s is specified, -s is assumed (i.e. leaf processes call ) and sFlag = 1
	flags.sleep_time = 0;
	int flag;
	while((flag = getopt(argc, argv, "uN:M:ps:")) != -1) {
		switch(flag) {
			case 'u':
				flags.uFlag = 1;
				perror(flags.usageString);
				exit(0);
				break;
			case 'N':
				if(atoi(optarg) == NULL) { perror("no <num_levels> argument specified\n"); exit(0); }
				flags.nFlag = atoi(optarg);
				if(flags.nFlag < 1 || flags.nFlag > 4) {
					perror("1 <= <number of levels> <= 4.\n");
					exit(0);
				}
				break;
			case 'M':
				if(atoi(optarg) == NULL) { perror("no <num_children> argument specified\n"); exit(0); }
				flags.mFlag = atoi(optarg);
				if(flags.mFlag < 1 || flags.mFlag > 3) {
					perror("1 <= <number of children> <= 3.\n");
					exit(0);
				}
				break;
			case 'p':
				flags.pFlag = 1;
				break;
			case 's':
				if(atoi(optarg) == NULL) { perror("no <sleep_time> argument specified\n"); exit(0); }
				flags.sFlag = 1;
				flags.sleep_time = atoi(optarg);
				break;
			default:
				perror(flags.usageString);
				exit(0);
				break;
		}
	}
}
