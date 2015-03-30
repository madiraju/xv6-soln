#ifndef __PROCESS_H
#define __PROCESS_H

#include<sys/defs.h>
#include<sys/page.h>

#define KERNBASE 0xffffffff80000000ul
#define NPROC 64				/* maximum number of processes */
#define KSTACKSIZE 4096			/* size of per-process kernel stack */
#define NOFILE 16				/* open files per process */

uint32_t proc_count;

/* process states */
enum procstate{

	UNUSED,
	EMBRYO,
	SLEEPING,
	RUNNABLE,
	RUNNING,
	ZOMBIE
};

struct context{
	uint64_t rbx;
	uint64_t rsp;
	uint64_t rbp;
	uint64_t r12;
	uint64_t r13;
	uint64_t r14;
	uint64_t r15;
};

struct trapframe{
/* trapframe generated by isr_common */
	uint64_t rbp;
	uint64_t r15;
	uint64_t r14;
	uint64_t r13;
	uint64_t r12;
	uint64_t r11;
	uint64_t r10;
	uint64_t r9;
	uint64_t r8;
	uint64_t rdi;
	uint64_t rsi;
	uint64_t rdx;
	uint64_t rcx;
	uint64_t rbx;
	uint64_t rax;

/* change in idt.c */
	/* uint64_t gs; */
	/* uint64_t fs; */
	/* uint64_t es; */
	/* uint64_t ds; */

	uint64_t intr_num;
	uint64_t error_code;

	uint64_t rip;
	uint64_t cs;
	uint64_t rflags;
	uint64_t rsp;
	uint64_t ss;
};

/* PCB - process control block */
struct proc{
	uint64_t size;				/* process memory size in bytes */
	pml4 *pml4_t;				  /* pointer to pml4. */
	char *kstack;				  /* pointer to start of stack (i.e bottom of stack) */
	enum procstate state;		  /* process state */
	int pid;					  /* process identifiers */
	struct proc *parent;		  /* parent of process */
	struct trapframe *tf;		  /* stack trapframe */
	struct context *context;	  /* kernel stack */
	void *chan;					  /* if non-zero, sleeping on chan */
	int killed;					  /* if non-zero then killed */
	/* struct file *ofile[];		  /\* list of open files *\/ */
	char name[32];				  /* process name */
};

uint64_t get_virt_addr(uint64_t x);

uint64_t get_phys_addr(uint64_t x);

void userinit();
struct proc * alloc_proc();

#endif

/* typedef struct register_t{ */

/* uint64_t rax; */
/* uint64_t rbx; */
/* uint64_t rcx; */
/* uint64_t rdx; */
/* uint64_t rsi; */
/* uint64_t rdi; */
/* uint64_t r8; */
/* uint64_t r9; */
/* uint64_t r10; */
/* uint64_t r11; */
/* uint64_t r12; */
/* uint64_t r13; */
/* uint64_t r14; */
/* uint64_t r15; */
/* uint64_t rbp; */
/* }reg_t; */

/* typedef struct task{ */

/* 	uint64_t ppid; 				/\* parent process id *\/ */
/* 	uint64_t pid;				/\* process id *\/ */
/* 	uint64_t *stack;			/\* user level stack virtual address *\/ */
/* 	reg_t regs;					/\* saved GPR of the process *\/ */
/* 	uint64_t rsp;				/\* stack pointer of user stack*\/ */
/* 	uint64_t cr3; */
/* 	/\* uint64_t rip; *\/ */
/* 	pml4 *pml4_p;				/\* pml4 structure virtual address *\/ */
/* 	void (* entry)();				/\* user function entry point *\/ */
/* 	uint64_t state;				/\* 0:ready, 1:wait, 2:sleep, 3:zombie *\/ */
/* 	uint64_t sleep_time;		/\* sleep time *\/ */
/* 	struct task *next; */
	
/* } pcb; */


/* typedef struct run_queue{ */

/* 	pcb *head; */
/* 	pcb *tail; */
/* 	pcb *cur_pcb;				/\* must implement this as a circular linked list *\/ */
/* }run_q; */


/* run_q run_q_p; */

/* uint64_t get_virt_addr(uint64_t x); */

/* uint64_t get_phys_addr(uint64_t x); */

/* void init_regs(reg_t *p); */

/* int create_process(); */

/* void proc_entry(); */

/* void init_runq(); */

/* void switch_to_next(); */
/* #endif */
