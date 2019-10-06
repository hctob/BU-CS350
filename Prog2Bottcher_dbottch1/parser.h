#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>

struct flag_data_t {
	char uFlag;
	char* usageString;
	char nFlag;
	char mFlag;
	char pFlag;
	char sFlag;
	int sleep_time;
};

extern struct flag_data_t flags;

void c_parser(int argc, char **argv);
