#include <stdio.h>
#include <stdlib.h>
#include <sys/ptrace.h>
#include <libunwind-ptrace.h>

static void *upt_info;
static unw_addr_space_t addr_space;


int unwind_initialize(int pid) {

    upt_info = _UPT_create(pid);   /* use thread id instead in a more comprehensive version */

     if ((addr_space = unw_create_addr_space(&_UPT_accessors, 0)) == NULL) {
        fprintf(stderr, "failed to create address space for unwinding\n");
        return -1;
     }

}

int unwind_cleanup() {

   unw_destroy_addr_space(addr_space);

   _UPT_destroy(upt_info);

   return 0;
}


int unwind(int pid)
{

    unw_cursor_t cursor;
    unw_word_t ip, off;
    static char buf[512];
    size_t len;
    int rc = 0, ret;

    if (unw_init_remote(&cursor, addr_space, upt_info) < 0) {
        fprintf(stderr, "failed to init cursor for unwinding\n");
        return -1;
    }

    do {
  
       buf[0] = '\0';
       if (unw_get_proc_name(&cursor, buf, sizeof(buf), &off) < 0) 
          break;
       printf("%s\n", buf);

     } while (unw_step(&cursor) > 0);
 

   return 0;

}


int detach_process(int pid)
{
    if (ptrace(PTRACE_DETACH, pid, NULL, NULL) < 0) {
        perror("detach");
        return -1;
    }
}

int attach_process(int pid)
{
    int status;

    if (ptrace(PTRACE_ATTACH, pid, NULL, NULL) < 0) {
        perror("attach");
        detach_process(pid);
        return -1;
    }
    waitpid(pid, &status, 0);
    while (!WIFSTOPPED(status)) {
        usleep(1);
        waitpid(pid, &status, 0);
    }

}

