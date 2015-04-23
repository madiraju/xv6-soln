#include <sys/sbunix.h>
#include<sys/syscall.h>
#include<stdlib.h>

int main(int argc, char* argv[], char* envp[]) {
	printf("hello\n");
	int i;
	int copy=10;
	for(i=0;argv[i];i++){
		printf("argv[%d]->%s\n",i,argv[i]);
	}
	for(i=0;envp[i];i++){
		printf("envp[%d]->%s\n",i,envp[i]);
	}
	int status=0;
	pid_t b=fork();
	int pid=0, pid1=0;
	if(b>0){
		printf("parent says hi\n");
		printf("parent waiting for child process %d\n",b);
		printf("argc is %d\n",argc);
		/* pid=waitpid(-1,&status,0); */
		yield();
	}
	else if(b==0){
		pid_t b1=fork();
		if(b1>0){
			printf("child_parent says hi\n");
			printf("child_parent's child %d\n",b1);
			printf("copy in child is %d\n",copy);
			printf("argc in child %d\n", argc);
			for(i=0;argv[i];i++){
				printf("child par argv[%d]->%s\n",i,argv[i]);
			}
			for(i=0;envp[i];i++){
				printf("chile par envp[%d]->%s\n",i,envp[i]);
			}
			pid1=waitpid(b1,&status,0);
			printf("child_parent %d exited\n",pid1);
			printf("child_parent about to exit\n");
			exit(0);
		}
		else if(b1==0){
			pid_t b2=fork();
			if(b2>0){
				printf("child_child says hi\n");
				printf("child_child about to exit\n");
				for(i=0;argv[i];i++){
					printf("child argv[%d]->%s\n",i,argv[i]);
				}
				execve("bin/exec", argv, envp);
				for(i=0;envp[i];i++){
					printf("child envp[%d]->%s\n",i,envp[i]);
				}
				sleep(10);
				exit(0);
			}
			else if(b2==0){
				execve("bin/exec", argv, envp);
			}
		}
	}
	printf("parent child exited %d\n",pid);
	while(1)
		yield();
	return 0;
}

