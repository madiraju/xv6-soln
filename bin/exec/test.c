#include<stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[], char* envp[]) {
	printf("test exec successful\n");
	perror("test after sleep\n");
	return 0;
}

