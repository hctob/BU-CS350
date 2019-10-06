#include <iostream>
#include <string>
#include <list>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>


unsigned int n_count, num_writers, num_readers;
pthread_mutex_t read_lock = PTHREAD_MUTEX_INITIALIZER, write_lock = PTHREAD_MUTEX_INITIALIZER, readTry = PTHREAD_MUTEX_INITIALIZER, resource = PTHREAD_MUTEX_INITIALIZER, cond_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cv = PTHREAD_COND_INITIALIZER;
unsigned int read_count, write_count, almost_done;

//based off of semaphore solution to Readers-writers problem off of Wikipedia

typedef struct __myargs_t{
	int num;
} myargs_t;

typedef struct __node_t {
	int value;
	struct __node_t *next;
} node_t;

typedef struct __list_t {
	node_t *head;
	node_t *tail;
} list_t;

list_t listr = (list_t){nullptr, nullptr};

void add(int num){
	node_t *new_node = (node_t*) malloc(sizeof(node_t));
	new_node->value = num;
	if(listr.head == NULL){
		listr.head = new_node;
		listr.tail = new_node;
	}

	else{
		listr.tail->next = new_node;
		listr.tail = listr.tail->next;
	}
	
	return;
}

int read(int num){
	int retVal = 0;
	node_t *temp = listr.head;
	while(temp != NULL){
		if(temp->value % 10 == num){
			retVal++;
		}
		temp = temp->next;
	}
	return retVal;
}

void * reader(void* param) {
	myargs_t *m = (myargs_t *)param;
	int i = m->num; //n->num never getting set/incremented
	FILE* fp; //file pointer for I/0
	char derp[16];
	const char* _derp = "reader_";
	snprintf(derp, 16, "%s%d.txt", _derp, i); //10 characters + (1-4 characters for N) + 2 extra bytes for null-terminating character
	fp = fopen(derp, "w+");

	unsigned int y;
	for(y = 1; y <= n_count; y++){
		//entry section
		//printf("Reader %d entering\n", i);
		pthread_mutex_lock(&readTry);
		pthread_mutex_lock(&read_lock); //slap the padlock on the pantry
		read_count++;
		if(read_count == 1) {
			//if first reader entering, lock resource(?)
			pthread_mutex_lock(&resource);
		
		}

		
		pthread_mutex_unlock(&read_lock); //finally remember where my keys are
		pthread_mutex_unlock(&readTry);

		//CRITICAL SECTION - ACTUAL READING/WRITING TO OUTPUT
	
		int k = read(i);

		fprintf(fp, "Reader %d: Read %d: %d values ending in %d\n", i, y, k, i); //file I/O, where i = num reader, y = num reads, k = sum of counts


		//exit section
		pthread_mutex_lock(&read_lock); //slap the padlock on the pantry
		read_count--;
		if(read_count == 0) {
			//if last reader leaving, unlock resource (?)
			pthread_mutex_unlock(&resource);
		}
		pthread_mutex_unlock(&read_lock);
		//printf("Reader %d leaving\n", i);
		
		struct timespec tim;
		tim.tv_sec = 0;
		tim.tv_nsec = 600000;	
		nanosleep(&tim, NULL);

	}

	pthread_mutex_lock(&cond_lock);
	//printf("%d\n", almost_done);
	almost_done--;
	pthread_cond_signal(&cv);
	pthread_mutex_unlock(&cond_lock);

	fclose(fp);

	return NULL;

}

void * writer(void* param) {
	myargs_t *m = (myargs_t *) param;
	int i = m->num;
	unsigned int k;
	for(k = 1; k <= n_count; k++){
		//entry section
		pthread_mutex_lock(&write_lock);
		//printf("Writer %d entering\n", i);
		write_count++;
		if(write_count == 1){
			pthread_mutex_lock(&readTry);
		}
		pthread_mutex_unlock(&write_lock);

		pthread_mutex_lock(&resource); 


		//writing being performed
		//srand(time(NULL));
		int dataToWrite = (rand() % 100) + 1;
		dataToWrite = dataToWrite*10;
		dataToWrite += i;
		add(dataToWrite);

		pthread_mutex_unlock(&resource);


		//exit section
		//printf("Writer %d leaving\n", i);
		pthread_mutex_lock(&write_lock);
		write_count--;
		if(write_count == 0){
			pthread_mutex_unlock(&readTry);
		}
		pthread_mutex_unlock(&write_lock);

		struct timespec tim;
		tim.tv_sec = 0;
		tim.tv_nsec = 700000; //found this to be an optimal amount to nanosleep
		nanosleep(&tim, NULL);

	}

	return NULL;

}

void * check_almost_done(void* param){
	pthread_mutex_lock(&cond_lock);
	while(almost_done > 1){
		pthread_cond_wait(&cv, &cond_lock);
	}
	printf("Almost Done!\n");
	pthread_mutex_unlock(&cond_lock);
	
	return NULL;
}

int main(int argc, char** argv, char** envp) {
	if(argc < 4) {
		printf("Usage: readwrite N R W\n");
		exit(1);
	}
	srand(time(NULL));
	read_count = 0; //initial value = 0
	write_count = 0; //initial value = 0

	n_count = atoi(argv[1]); //number of reads/numbers to be written (1 - 1000 val inclusive)
	num_readers = atoi(argv[2]); //number of readers, 1 - 9 inclusive
	num_writers = atoi(argv[3]); //number of writers, 1 - 9 inclusive
	if(n_count > 99 || n_count < 1 || num_writers > 9 || num_writers < 1 || num_readers > 9 || num_readers < 1) { 
		exit(1); 
	}

	almost_done = num_readers;
	
	pthread_t * readers = new pthread_t[num_readers];
	pthread_t * writers = new pthread_t[num_writers];
	myargs_t * arguments = new myargs_t[9];

	pthread_t done_checker;

	unsigned int i;
	//srand(time(NULL));
	//number of threads

	for(i = 1; i <= num_readers; i++) {
		arguments[i-1].num = i;
		readers[i-1] = pthread_create((readers + i - 1), NULL, reader, (arguments + i - 1));
		//pthread_join(readers[i-1], NULL);
	}
	//printf("Readers are done making\n");
	for(i = 1; i <= num_writers; i++) {
		arguments[i-1].num = i;
		writers[i-1] = pthread_create((writers + i - 1), NULL, writer, (arguments + i - 1));
		//pthread_join(writers[i-1], NULL);
	}

    pthread_create(&done_checker, NULL, check_almost_done, NULL);
    pthread_join(done_checker, NULL);
	//printf("Writers are done making\n");
	for(i = 0; i < num_readers; i++) {
		//arguments[i].num = i;
		pthread_join(readers[i], NULL);
	}
	for(i = 0; i < num_writers; i++) {
		//arguments[9+i].num = i;
		pthread_join(writers[i], NULL);
	}
	//printf("Joining is done joining\n");


	return 0;
}
