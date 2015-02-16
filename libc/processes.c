#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/syscall.h>
#include <syscall.h>

struct timespec{
    time_t tv_sec;
    long tv_nsec;
};

pid_t fork(void) {
	return  (pid_t) syscall_0(SYS_fork);
}

pid_t getpid(void) {
	return  (pid_t) syscall_0(SYS_getpid);
}

pid_t getppid(void) {
	return  (pid_t) syscall_0(SYS_getppid);
}

int execve(const char *filename, char *const argv[], char *const envp[]) {
  
  int ret= (int) syscall_3(SYS_execve, (uint64_t) filename, (uint64_t) argv, (uint64_t) envp);
  if(ret < 0)
    return -1;
  return 0;
}

pid_t waitpid(pid_t pid, int *status, int options) {
	return (pid_t) syscall_3(SYS_wait4, (uint64_t) pid, (uint64_t) status, (uint64_t) options);
}

unsigned int alarm(unsigned int seconds) {
	return  (unsigned int) syscall_1(SYS_alarm, (uint64_t) seconds); 
}

unsigned int sleep(unsigned int seconds){
    struct timespec req;
    unsigned int ret;
    req.tv_sec=seconds;
    req.tv_nsec=0L;
    struct timespec rem;
    ret=syscall_2(SYS_nanosleep,(uint64_t)&req,(uint64_t)&rem);
    return ret;
}

