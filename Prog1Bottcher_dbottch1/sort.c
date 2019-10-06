#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "parse.h"
/*void count_username_chars(int size, char* input, int* derp) {
}*/

struct global_t g;

int compare (const int * a, const int * b)
{
  return ( *a - *b );
}

int main(int argc, char **argv, char *envp[]) {	
	time_t p_beg;
	time_t p_end;
	FILE *op, *ip, *cp;
	c_parser(argc, argv);
	p_beg = clock();
	char* user_name_up; //unparsed username char*
	user_name_up = envp[9];
	//printf("%s\n", user_name_up); //envp[9] = USER=username
	//malloc crap
	char* username_parsed;
//get substring
//get size of username 
	username_parsed = user_name_up += 5;
	//sprintf(username_parsed, "%s", user_name_up + 5); //parse username to remove 
	//printf("%s\n", username_parsed);
	//char t = username_parsed[0];
	unsigned int length = (unsigned)strlen(username_parsed);
	username_parsed[length] = '\0';
	//printf("check a\n");
	//printf("%u\n", length);
	int* derp = malloc((unsigned)length * sizeof(int)); //
	if(derp == NULL) {
		perror("Error with malloc of derp [count]\n"); 
		exit(1);
	}
	//printf("check b\n");
	memset(derp, 0, length * sizeof(int));
	//p_beg = clock(); //clock starts
	int* sorted_nums = (int*)malloc(sizeof(int) * g.nFlag);
	if(g.iFlag == 1) {
		//printf("iFlag set\n");
		ip = fopen(g.ip, "r");
		if(ip == NULL) {
			perror("File not opened correctly.\n");
			exit(1);
		}
		//printf("input file opened\n");
		//printf("nFlag = %d\n", g.nFlag);
		for(int i = 0; i < g.nFlag; i++) {
			int tmp = 0;
			//printf("%d ", i);
			fscanf(ip, "%d", &tmp);
			printf("tmp: %d\n", tmp);
			for(int o = 0; o < length; o++) {
				if(tmp == username_parsed[o]) {
					printf("%s encountered\n", username_parsed[o]);
					derp[o] += 1;
				}
			}
			sorted_nums[i] = tmp;
		}
		//printf("array populated\n");
		fclose(ip);
		//printf("file closed\n");
	}
	else {
		//printf("iFlag not set\n");
		//printf("Reading from standard input: \n");
		int count = 0;
		int tmp;
		while(scanf("%d", &tmp) == 1) {
			sorted_nums[count] = tmp;
			count++;
		}
	}
	printf("beginning sorting\n");
	qsort(sorted_nums, g.nFlag, sizeof(sorted_nums[0]), compare);
	if(g.oFlag == 0) {
		for(int j = 0; j < g.nFlag; j += 1) {
			printf("Sorted nums[%d]: %d\n", j, sorted_nums[j]);
		}	
	}
	else {
		op = fopen(g.op, "w");
		for(int j = 0; j < g.nFlag; j += 1) {
			fprintf(op, "Sorted nums[%d]: %d\n", j, sorted_nums[j]);
		}
	}
	printf("sorted\n");
	//int line_count = 0;
	//int* sorted_nums = (int*)malloc(sizeof(int));
	/*if(g.iFlag == 1) {
		ip = fopen(g.ip, "r");
		char* s;
		while(fscanf(ip, "%s", s) != EOF) {
			sorted_nums = (int*)realloc(sorted_nums, (line_count + 1) * sizeof(int));
			int num;
			fscanf(ip, "%d", &num);
			sorted_nums[line_count] = num;
			line_count++;
		}
		fclose(ip);
	}
	else {*/
	/*int tmp;
	while(scanf("%d", &tmp) != EOF) {
			//sorted_nums = (int*)realloc(sorted_nums, (line_count + 1) * sizeof(int));
			//sorted_nums[line_count] = tmp;
		printf("%d\n", tmp);
		line_count++;
	}*/
	
	//qsort(sorted_nums, sizeof(sorted_nums)/sizeof(int), sizeof(int), compare);
	printf("done\n");
	
	/*if(g.oFlag == 0) {
		for(int i = 0; i < sizeof(sorted_nums) / sizeof(int); i += 1) {
			printf("%d\n", sorted_nums[i]);
		}
	}*/
	if(g.cFlag == 0) {
		for(int k = 0; k < length; k++) {
			printf("%c %d %d\n", username_parsed[k], (int)username_parsed[k], derp[k]);
		}
		//printf("%c %d %d\n", username_parsed[0], (int)username_parsed[0], derp[0]);
		/*printf("%c %d %d\n", username_parsed[1], (int)username_parsed[1], derp[1]);
		printf("%c %d %d\n", username_parsed[2], (int)username_parsed[2], derp[2]);
		printf("%c %d %d\n", username_parsed[3], (int)username_parsed[3], derp[3]);
		printf("%c %d %d\n", username_parsed[4], (int)username_parsed[4], derp[4]);
		printf("%c %d %d\n", username_parsed[5], (int)username_parsed[5], derp[5]);
		printf("%c %d %d\n", username_parsed[6], (int)username_parsed[6], derp[6]);
		printf("%c %d %d\n", username_parsed[7], (int)username_parsed[7], derp[7]);*/
	}
	else {
		cp = fopen(g.cp, "w");
		for(int k =0; k < length; k++) {
			fprintf(cp, "%c %d %d\n", username_parsed[k], (int)username_parsed[k], derp[k]);
		}
		/*fprintf(cp, "%c %d %d\n", username_parsed[0], (int)username_parsed[0], derp[0]);
		fprintf(cp, "%c %d %d\n", username_parsed[1], (int)username_parsed[1], derp[1]);
		fprintf(cp, "%c %d %d\n", username_parsed[2], (int)username_parsed[2], derp[2]);
		fprintf(cp, "%c %d %d\n", username_parsed[3], (int)username_parsed[3], derp[3]);
		fprintf(cp, "%c %d %d\n", username_parsed[4], (int)username_parsed[4], derp[4]);
		fprintf(cp, "%c %d %d\n", username_parsed[5], (int)username_parsed[5], derp[5]);
		fprintf(cp, "%c %d %d\n", username_parsed[6], (int)username_parsed[6], derp[6]);
		fprintf(cp, "%c %d %d\n", username_parsed[7], (int)username_parsed[7], derp[7]);*/
		fclose(cp);
	}
	/*for(i = 0; i < 4; i++) {
		printf("derp[%d] = %d\n", i, derp[i]);
	}*/
	/*for(int i = 0; i < 2; i++) {
		char c = username_parsed[i];
		printf("%c, %u, %u", c, 0, 0);
	}*/
	//printf("%s\n", user_name_up); //envp[9] = USER=username
	free(derp);
	free(sorted_nums);
	p_end = clock();
	if(g.oFlag == 1) {
		fprintf(op, "Time elapsed: %f", (double)(p_end - p_beg) / (double)CLOCKS_PER_SEC);
		fclose(op);
	}
	return 0;
}

