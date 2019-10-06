#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include <stdlib.h>
#include "parser.h"

struct flag_data_t flags;

int main(int argc, char **argv, char **envp) {
	//bool root_leaf = false;
	char root = 0;
	c_parser(argc, argv); //parse all values
	if(flags.nFlag == 0) flags.nFlag = 1;
	if(flags.pFlag == 1 && flags.sFlag == 1) {
		//first, check if sFlag has been set
		perror("Cannot pause and sleep a process\n");
		//printf("Cannot pause and sleep a process\n");
		perror(flags.usageString);
		exit(0); 
	}
	else if(flags.pFlag == 0 && flags.sFlag == 0) {
		//pFlag and sFlag not set
		root = 1;
		flags.sleep_time = 1;
	}
	printf("ALIVE: Level [%d] process with p_id=[%d], child of pp_id=[%d]\n", (int)flags.nFlag, (int)getpid(), (int)getppid());

	if(flags.nFlag == 0 || flags.nFlag == 1) { 
		//should be root level one process(??)
		if(root) {
			sleep(1);
		}
		if(flags.pFlag == 1) {
			pause(); //if -p is set, pause 
		}
		else if(flags.sFlag == 1) {
			sleep(flags.sleep_time); //if -s is set, sleep by the specified argument amount
		}
		else {
			wait(NULL); //wait for child processes to complete
		}
	}
	else if(flags.nFlag > 1) {
		//
		unsigned int q;
		for(q = 0; q < flags.mFlag; q++) {
			pid_t p_id = fork();
			if(p_id < 0) { 
				//child failed
				perror("Fork failed.\n");
				//printf("Fork failed\n");
				exit(0);
			}
			else if(p_id == 0) {
				//child exists (oh boy!!)
				//printf("Fork worked\n");
				//execvp("prog_name", prog_args); first argument, by convention, is a char* to filename of program.
				//prog args must be null-terminated char* and final char* must be NULL
				char* num_levels = (char*)malloc(sizeof(char) * 2);
				char* num_children = (char*)malloc(sizeof(char) * 2);
				char* sleep_time = (char*)malloc(sizeof(char) * 16);
				snprintf(num_levels, 16, "%d", flags.nFlag - 1);
				snprintf(num_children, 16, "%d", flags.mFlag);
				//sprintf(num_levels, "%d", flags.nFlag - 1); //semi-recursive way of NOT making a fork bomb lol //no need to add \0 as string is already null terminated
				//sprintf(num_children, "%d", flags.mFlag); //no need to add \0 as string is already null terminated
				char* child_args[9] = {"./prog2tree", "-N", num_levels, "-M", num_children, NULL};
				//printf("%s %s %s %s %s %s\n", child_args[0], child_args[1], child_args[2], child_args[3], child_args[4], child_args[5]);
				//printf("test\n");
				if(flags.pFlag == 1) {
					child_args[5] = "-p";
				}
				else if(flags.sFlag == 1) {
					child_args[5] = "-s";
					snprintf(sleep_time, 64, "%d", flags.sleep_time);	
					//sprintf(sleep_time, "%d", flags.sleep_time); //no need to add \0 as string is already null terminated
					child_args[6] = sleep_time;
				} 
				//execvp(argv[0], child_args);
				execvp("./prog2tree", child_args);
				//printf("Exec success!\n"); only would print if execvp fails
			}
			//wait(NULL); //this will wait until child process is completed before parent continues along 
		}
		/*unsigned int batman_symbol = 0;
		while(batman_symbol < flags.mFlag) {
			wait(NULL);
		}*/
		for(q = 0; q < flags.mFlag; q++) wait(NULL);
	}
	printf("EXITING: Level [%d] process with p_id=[%d], child of pp_id=[%d]\n", (int)flags.nFlag, (int)getpid(), (int)getppid());
	return 0;
}
	
