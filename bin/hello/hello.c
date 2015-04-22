#include <sys/sbunix.h>
#include<sys/syscall.h>
#include<stdlib.h>

int main(int argc, char* argv[], char* envp[]) {
	printf("hello\n");
	pid_t b=fork();
	if(b>0){
		printf("parent says hi\n");
		sleep(8);
	}
	else if(b==0){
		pid_t b1=fork();
		if(b1>0){
			printf("child_parent says hi\n");
			yield();
			sleep(2);
			printf("child_parent wokeup\n");
		}
		else if(b1==0){
			printf("child_child says hi\n");
		}
		while(1){
			yield();
		}
	}
	printf("parent wokeup\n");
	while(1);
	return 0;
}

