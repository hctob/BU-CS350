#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
int main() {
	pid_t pid; /* for a child process */
	pid = fork();
	if (pid < 0) { /* error */
		fprintf(stderr, "Fork failed.");
		return 1;
	}
 	if (pid == 0) { /* child process */
 		execlp("/bin/ls", "ls", NULL);
 	}
 	else { /* parent process */
 		/* parent will wait for the child to complete */
 		wait(NULL);
 		printf("Child Complete.");
 	}
	return 0;
}
