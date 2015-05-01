#include<stdlib.h>
#include<stdio.h>
#include<string.h>


int main(int argc, char* argv[], char* envp[]) {
	printf("hello\n");
	int i;
	for(i=0;argv[i];i++){
		printf("argv[%d]->%s\n",i,argv[i]);
	}
	for(i=0;envp[i];i++){
		printf("envp[%d]->%s\n",i,envp[i]);
	}
	int status=0,pid=0;
	int fd[2];
	if(pipe(fd)==-1){
		printf("pipe failed\n");
	}
	pid_t b=fork();
	char *x="pipe content!!!\n";
	if(b>0){
		printf("parent says hi\n");
		close(fd[0]);			/* parent closing read end */
		write(fd[1],x,strlen(x));
		yield();
		printf("parent sleeps\n");
		sleep(4);
		printf("parent wakes up\n");
		char *y="extra content\n";
		int ret=write(fd[1],y,strlen(y));
		printf("parent valid write returns %d\n",ret);
		ret=write(fd[0],y,strlen(y));
		printf("parent invalid write returns %d\n",ret);
		pid=waitpid(-1,&status,0);
		char *a=(char *)malloc(0x10000*sizeof(char));
		printf("enter string\n");
		scanf("%s",a);
		printf("a->%s\n",a);
		yield();
	}
	else if(b==0){
		int k=fork();
		if(k>0){
			printf("child says hi\n");
			close(fd[1]);		/* child closing write end */
			char c[1000];
			int len;
			printf("1 read %d bytes\n",(len=read(fd[0],c,strlen(x)-5)));
			c[len]='\0';
			printf("child read %s\n",c);
			printf("2 read %d bytes\n",(len=read(fd[0],c,strlen(x))));
			c[len]='\0';
			printf("child read %s\n",c);
			printf("3 read %d bytes\n",(len=read(fd[0],c,strlen(x))));
			c[len]='\0';
			printf("child read %s\n",c);
			execve("bin/exec",argv,envp);
			exit(0);
		}
		else if(k==0){
			printf("child child says hi\n");
			while(1)
				yield();
		}
	}
	printf("parent child exited %d && %d\n",b,pid);
	while(1)
		yield();
	return 0;
}

