#include<sys/sbunix.h>
#include<sys/utility.h>
#include<sys/process.h>
#include<sys/page.h>
#include<sys/pmmgr.h>
#include<sys/syscall.h> 

extern void isr128();

#define LSTAR  0xC0000082
#define SFMASK 0xC0000084
#define STAR   0xC0000081
#define EFER   0xC0000080
#define LO_MASK 0x00000000FFFFFFFFul
#define HI_MASK 0xFFFFFFFF00000000ul

void init_syscall(){

  /* activate extended syscall instructions */
  /* set EFER's bit [0] SCE  */
  __asm__ __volatile__("wrmsr" : : "a"(0x1), "d"(0x0), "c"(EFER));

  /* for SYSCALL */
  /* set up STAR's bits [32-47] SYSCALL CS AND SS -> KCS,KDS */
  /* [32-16]->0x0(change if using sysret) */
  /* [16-8]->KCS->0x08 */
  /* [8-0]->KDS->0x10 */
  uint32_t hi=0x00000810;
  __asm__ __volatile__("wrmsr" : : "a"(0x0), "d"(hi), "c"(STAR));
  /* set up LSTAR with address of isr128 */
  __asm__ __volatile__("wrmsr" : : "a"(((uint64_t)&isr128) & LO_MASK), "d"((((uint64_t)&isr128) & HI_MASK)>>32), "c"(LSTAR));
  /* set up SFMASK with [0-31] with Rflags clear bit -> FL_IF */
  __asm__ __volatile__("wrmsr" : : "a"(FL_IF), "d"(0x0), "c"(SFMASK));
}


void do_yield(){
	proc->state=RUNNABLE;
	/* printf("proc -> %d -> yield syscall\n",proc->pid); */
	sched();
	
}

void do_fork(){
	/* alloc_proc finds a spot in pcb array and allocates kstack and sets sp and tf pointers and gives a pid*/
	printf("proc -> %d -> fork syscall\n",proc->pid);
	struct proc *p=alloc_proc();
	if(!p){
		/* p is null, alloc_proc couldnt find a spot in pcb array */
		printf("unable to fork. no space for new processs\n");
		/* put -1 in proc->tf->rax, this tells parent that fork failed */
		proc->tf->rax=-1;		/* syscall_0 returns "eax"(that's right) as uint32_t to fork(), in user program we cast to int and then check for -1 */

		/* such as kernel stack and set pcb of p to UNUSED  */
		return ;
	}

	/* copyuvm deallocates page table memory on failure, within itself */
	/* copy parent procs address space into child */
	if(!(p->pml4_t = copyuvm(proc->pml4_t))){
		/* p->pml4_t is NULL, copyuvm failed */
		printf("unable to fork. no space for new process's page tables\n");
		/* put -1 in proc->tf->rax, this tells parent that fork failed */
		proc->tf->rax=-1;		/* syscall_0 returns "eax"(that's right) as uint32_t to fork(), in user program we cast to int and then check for -1 */

		/* if any failure in cpoyuvm or copyvma, then deallocate p's resources */
		/* such as kernel stack and set pcb of p to UNUSED   */
		free_pcb(p);
		return ;

	}

	/* copy VMAs from parent to child */
	if((p->vma_head=copyvma(proc->vma_head))==NULL){
		/* copyvma failed */
		printf("unable to fork. no space for new processes vmas\n");
		proc->tf->rax=-1;
		/* copyvma takes care of freeing new procs vmas */
		/* if any failure in copyuvm or copyvma, then deallocate p's resources */
		/* such as kernel stack and set pcb of p to UNUSED and free_uvm(p->pml4)*/
		free_uvm(p->pml4_t);
		free_pcb(p);
		return ;
	}
	

	/* copy trapframe of parent to child, do not clear trapframe */
	memcpy(p->tf,proc->tf,sizeof(struct trapframe));
	/* set return of fork of child to 0 */
	p->tf->rax=0;
	/* set size of child same as parent */
	p->size=proc->size;
	/* set parent of child to proc */
	p->parent=proc;
	/* give child the same name as parent */
	strcpy(p->name,proc->name);
	/* return pid of child to parent */
	proc->tf->rax=p->pid;
	/* set state of child to runnable */
	p->state=RUNNABLE;
	/* flush TLB */
	load_base(get_phys_addr((uint64_t)proc->pml4_t));
}
size_t do_write(int fd, const void* bf, size_t len){
	
	size_t i=0;
	const char *buf=(const char *)bf;
	if(fd==1){
		for(i=0;i<len;i++){
			printf("%c",buf[i]);
		}
	}
	return i;
} 
void do_brk(void* end_data_segment){
  
  /* printf("proc -> %d -> brk syscall\n",proc->pid); */
  struct vma *t=proc->vma_head;
  struct vma *t_stack=NULL, *t_heap=NULL;
  while(t!=NULL){
    /*traverse throug the process vma to get the heap and stack vmas */
    if(t->flags & PF_GROWSUP){
      /*if flags is PF_GROWSUP then heap*/
      t_heap=t;
    }
    else if(t->flags & PF_GROWSDOWN){
      /*if flags is PF_GROWSDOWN then stack*/
      t_stack=t;
    }
    t=t->next;
  }



  if( ((uint64_t)end_data_segment >= t_heap->end) && ((uint64_t)end_data_segment < t_stack->start) ){
    /* check end_data_segment is greater than current heap end and lesser than the current stack start*/
    t_heap->end=(uint64_t)end_data_segment + 0x1ul;
  }
  else{
    /*all other cases are invalid*/
    /*return the current break pointer*/
    end_data_segment= (void *)(t_heap->end - 0x1ul);
}
  /* return end_data_segment in any case*/
  proc->tf->rax=(uint64_t)end_data_segment;
}

void do_exit(int status, struct proc *p){

	printf("proc -> %d -> exit syscall\n",proc->pid);
	/* free vmas free_vma_list(head) */
	free_vma_list(&(p->vma_head));
	
	/* free process page tables free_uvm(pml4_t) */
	free_uvm(p->pml4_t);
	
	/* update waitpid Q */
	update_waitpid_queue(p);

	/* free pcb */
	free_pcb(p);
	
	/* call the sched */
	sched();
}

pid_t do_getpid(){
  return (pid_t)(proc->pid);
}
pid_t do_getppid(){
  if(proc->parent == NULL){
    return 0;
  }
  else{
    return (pid_t)(proc->parent->pid);
  }
}

void do_nanosleep(struct timespec *req,struct timespec *rem){
	printf("proc -> %d -> sleep syscall\n",proc->pid);
	if(req->tv_sec >= 0){
		/* sleep time is >0 secs */
		/* set state to SLEEPING */
		proc->state=SLEEPING;
		/* copy req to rem */
		rem->tv_sec=req->tv_sec;
		rem->tv_nsec=req->tv_nsec;
		/* add <proc,rem> to sleep Q */
		if(enqueue_sleep(proc,rem)==0){
			/* enqueue failed */
			/* sleep returns -1 */
			proc->tf->rax=-1;
			/* make proc RUNNING */
			proc->state=RUNNING;
			return ;
		}
		/* enqueue success */
		/* schedule next process */
		sched();
	}
}

void do_waitpid(pid_t pid, int* status, int options){

  printf("proc -> %d -> waitpid syscall\n",proc->pid);
  /* change the process state to SLEEPING */
  proc->state=SLEEPING;
  /* add the process to a waitpid Q */
  if(enqueue_waitpid(proc,pid)==0){
    /* enqueue failed */
    /* waitpid returns -1 */
    proc->tf->rax=-1;
    /* make process running */
    proc->state=RUNNING;
    return;
  }
  /* enqueue success */
  /* schedule another process */
  sched();
}

void do_read(int fd, void* buf, size_t count){
	printf("proc -> %d -> read syscall\n",proc->pid);
	/* add check if buf is in any of VMA's */
	if(fd==STDIN){
		/* check if foreground proc flag is set */
		if(fgproc==proc){
			if(_stdin->proc!=NULL){
				printf("_stdin Q is not free, someother process in it which is not fgproc\n");
				while(1);
			}
			/* add it to stdin Q */
			_stdin->proc=proc;
			_stdin->count=count;
			_stdin->buf=buf;
			/* if the terembuf is full, then we copy the requested bytes to the process and return to the process */
			if(isBufFull==1){
				do_copy();
			}
			else{
				/* since the buffer is empty, wait till the stdin has received 1 line feed */
				_stdin->proc->state=SLEEPING;
				/* schedule next process */
				sched();
				/* this process is woken up when keyboard.c receives a line feed of chars into the stdin buffer */
			}
			/* deque the proc from the stdin Q*/
			_stdin->proc=NULL;

		}
		else{
			/* If Set Kill This Process */
			printf("Proc -> %d Not Foreground Proc. Killing It\n",proc->pid);
			do_exit(0,proc);
		}
	}
}

void do_copy(){

	if(termbuf_head + _stdin->count <= termbuf_tail){
		/* load waiting procs page table */
		load_base(get_phys_addr((uint64_t)_stdin->proc->pml4_t));
		tss.rsp0=(uint64_t)read_kstack;
		struct proc *p=proc;
		proc=_stdin->proc;
		memcpy(_stdin->buf,termbuf_head,_stdin->count);
		proc=p;
		tss.rsp0=(uint64_t)p->kstack+KSTACKSIZE;
		/* load current procs page tables */
		load_base(get_phys_addr((uint64_t)p->pml4_t));
		termbuf_head+=_stdin->count;
		_stdin->proc->tf->rax=_stdin->count;
	}
	else{
		/* user requested more than present in buffer */
		/* load waiting procs page table */
		load_base(get_phys_addr((uint64_t)_stdin->proc->pml4_t));
		tss.rsp0=(uint64_t)read_kstack;
		struct proc *p=proc;
		proc=_stdin->proc;
		/* return till termbuf_tail */
		memcpy(_stdin->buf,termbuf_head,(termbuf_tail - termbuf_head));
		proc=p;
		tss.rsp0=(uint64_t)p->kstack+KSTACKSIZE;
		/* load current procs page tables */
		load_base(get_phys_addr((uint64_t)p->pml4_t));
		/* handle page faults */
		termbuf_head+=(termbuf_tail - termbuf_head);
		_stdin->proc->tf->rax=(termbuf_tail - termbuf_head);
	}
	if(termbuf_head==termbuf_tail){
		/* all termbuf has been read */
		/* restart from begining */
		termbuf_head=termbuf_tail=termbuf;
		/* termbuf is empty */
		isBufFull=0;
	}
}


void do_pipe(int *fd_arr){
	printf("proc -> %d -> pipe syscall\n",proc->pid);
	/* allocate 2 FILE structs */
	struct file *rf,*wf;
	int fd0, fd1;
	if(pipealloc(&rf,&wf) < 0){
		proc->tf->rax=-1;
		return;
	}
	fd0=-1;
	/* put those pounters in local pcb ofiles fd table */
	if((fd0=fdalloc(rf)) < 0 || (fd1=fdalloc(wf)) < 0){
		/* fd alloc failed */
		printf("unable to allocate local fd\n");
		if(fd0 >= 0){
			proc->ofile[fd0]=NULL;
		}
		fileclose(rf);
		fileclose(wf);
		proc->tf->rax=-1;
		return ;
	}
	/* put index of local pcb ofiles fd table in fd_arr[0] and fd_arr[1] */
	fd_arr[0]=fd0;
	fd_arr[1]=fd1;
	proc->tf->rax=0;
	return ;
}
