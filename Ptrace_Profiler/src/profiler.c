#include <stdio.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <sys/uio.h>

double cpu_time_used;

int print_kernel_stack(int pid) 
{
    FILE *file;
    char file_name[128];
    char addr[512];
    char name[512];

    sprintf(file_name, "/proc/%d/stack", pid);
 
    if ((file = fopen(file_name, "r")) == NULL) {
        fprintf(stderr, "cannot open %s\n", file_name);
        return -1;
    }
    
    while (fscanf(file,"%s %s", addr, name) !=EOF)
        printf("%s\n", name);

    fclose(file);

    return 0;
}

char proc_stat(int pid)  
{
    FILE *file;
    char buf[128];
    char pidstring[128];
    char pname[128];
    char pstate[1];
    
    sprintf(buf, "/proc/%d/stat", pid);
    if ((file = fopen(buf, "r")) == NULL) {
        fprintf(stderr, "Cannot open %s\nNo permissions or pid=%d does not esist\n\n", buf, pid);
        exit(-1);
    }

    fscanf(file, "%s %s %s", pidstring, pname, pstate);

    fclose(file);
    return toupper(pstate[0]);
}


/* reads selected values from Oracle process memory (specificallay from X$KSUSE in SGA) */
/* for old kernels read from /proc/<pid>/mem should be used instead of SYS_process_vm_readv */
int read_ksuse(int pid, int *ksuseopc_addr, int *ksusetim_addr)
{
    struct iovec local[2];
    struct iovec remote[2];
    char buf1[4];
    char buf2[4];
    ssize_t nread;
    int ksusetim;
    int ksuseopc;
    
    local[0].iov_base = buf1;
    local[0].iov_len = 4;
    remote[0].iov_len = 4;
    remote[0].iov_base = ksuseopc_addr;

    local[1].iov_base = buf2;
    local[1].iov_len = 4;
    remote[1].iov_len = 4;
    remote[1].iov_base = ksusetim_addr;
    
    nread = syscall(SYS_process_vm_readv,pid, local, 2, remote, 2, 0ULL);
    if (nread != 8) {
        printf("Cannot read target process memory.\n");
        return -1;
    }

    ksuseopc=*(int *) buf1;
    ksusetim=*(int *) buf2;
    
    /* X$KSUSE.ksusetim is the time in microseconds since last wait. 
 *        When X$KSUSE.ksusetim >0 the session is not waiting 
 *               ignore edge cases of sessions transitioning state, 
 *                      i.e. X$KSUSE.ksusetim=0 (musec) and process just switched on CPU 
 *                             This is a workaround as one should rather check X$KSLWT.KSLWTINWAIT
 *                                  */
    if (ksusetim>0) {
        return 0; /* on CPU */
    } else {
        return ksuseopc; /*wait event number */
    }
    
}

int stack_profile(int pid, int sleep_micro, int num_samples, int *ksuseopc_addr, int *ksusetim_addr)
{
   int i;
   char procstate;
   int event;
   char command_stack[128];
   char command_status[128];
   
   sprintf(command_stack, "cat /proc/%d/stack", pid);
   sprintf(command_status, "grep -m 1 State /proc/%d/status", pid);
   
    if (unwind_initialize(pid) < 0)  {
        detach_process(pid);
        return -1;
    }
	
    for (i=0; i < num_samples; i++) {

        /* note, data collection is not atomic, this can introduce errors */

        if (system(command_stack) != 0)   /* read kernel stack */
            exit(-1);
			
		if (attach_process(pid) < 0)
            return -1;

		if (unwind(pid) < 0)  {
           detach_process(pid);
           return -1;
        }

        if (detach_process(pid) < 0)
           return -1;
	  
        fflush(stdout);
        if (system(command_status) != 0)  /* read process state */
            exit(-1);
       
        /* optionally read Oracle wait event info if the addresses are provided */
        if ((ksuseopc_addr != NULL) &&  (ksusetim_addr != NULL)) {

            if ((event = read_ksuse(pid, ksuseopc_addr, ksusetim_addr)) < 0) 
                return -1;

            if (event > 0) 
                printf("event#=%d (Oracle state)\n", event);
            else 
                printf("On CPU (Oracle state)\n");          
                            
        }
		
        printf("1\n\n");
        fflush(stdout);
		
        usleep(sleep_micro);
    }
            
    unwind_cleanup();
    return 0;
}



