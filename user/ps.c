// ps.c — userspace program to list all running xv6 processes
// Uses the sys_getprocs() syscall added to the kernel.

#include "kernel/types.h"
#include "kernel/pinfo.h"
#include "user/user.h"

static const char *state_names[] = {
  "UNUSED  ",
  "USED    ",
  "SLEEPING",
  "RUNNABLE",
  "RUNNING ",
  "ZOMBIE  ",
};

int
main(void)
{
  struct procinfolist pil;

  if (getprocs(&pil) < 0) {
    fprintf(2, "ps: getprocs failed\n");
    exit(1);
  }

  printf("PID\tNAME\t\tSTATE\t\tMEM(KB)\tPGFAULTS\tSYSCALLS\n");
  printf("----------------------------------------------------------------\n");

  for (int i = 0; i < pil.nproc; i++) {
    struct procinfo *pi = &pil.procs[i];
    const char *st = (pi->state >= 0 && pi->state <= 5)
                     ? state_names[pi->state] : "UNKNOWN ";
    printf("%d\t%s\t\t%s\t%d\t%d\t\t%d\n",
           pi->pid,
           pi->name,
           st,
           (int)(pi->sz / 1024),
           (int)pi->pagefaults,
           (int)pi->syscall_count);
  }
  printf("\n%d process(es) running.\n", pil.nproc);
  exit(0);
}
