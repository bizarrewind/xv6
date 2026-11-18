// vmbench.c
// tests lazy sbrk and cow fork
// basically allocate mem, touch some pages, fork and write, see how many faults happen

#include "kernel/types.h"
#include "kernel/vmstat.h"
#include "user/user.h"

// just a helper to print a line
void line() {
  printf("-------------------------------\n");
}

int
main(void)
{
  struct vmstat s1, s2;
  int pid;

  printf("\n== vmbench: testing lazy alloc and cow ==\n\n");

  // ---- test 1: lazy sbrk ----
  // request 100 pages but only touch 10
  // should only get 10 demand faults not 100

  printf("test 1: lazy sbrk\n");
  line();

  vmstat(&s1); // snapshot before

  int npages = 100;
  char *mem = sys_sbrk(npages * 4096, 0); // 0 = lazy mode
  if(mem == (char*)-1){
    printf("sbrk failed\n");
    exit(1);
  }

  // touch only 10 pages by writing to them
  int i;
  for(i = 0; i < 10; i++){
    mem[i * 4096] = 1; // write one byte to trigger page fault
  }

  vmstat(&s2); // snapshot after

  printf("  pages requested : %d\n", npages);
  printf("  pages we touched: 10\n");
  printf("  demand faults   : %d\n", (int)(s2.demand_faults - s1.demand_faults));
  printf("  pages saved     : ~%d (never touched so never allocated)\n", npages - 10);
  printf("  free pages now  : %d\n\n", (int)s2.free_pages);

  // ---- test 2: cow fork, kids only read ----
  // fork 4 kids, they all just read the pages
  // cow faults should be 0 because nobody is writing

  printf("test 2: cow fork read only\n");
  line();

  // first allocate some pages to share
  char *cowmem = sys_sbrk(50 * 4096, 0);
  for(i = 0; i < 50; i++)
    cowmem[i * 4096] = 'x'; // touch all 50 so they exist

  vmstat(&s1);

  int c;
  for(c = 0; c < 4; c++){
    pid = fork();
    if(pid == 0){
      // child just reads, shouldnt cause any cow faults
      char tmp = 0;
      int j;
      for(j = 0; j < 50; j++)
        tmp += cowmem[j * 4096];
      (void)tmp; // so compiler doesnt remove the reads
      exit(0);
    }
  }
  for(c = 0; c < 4; c++)
    wait(0);

  vmstat(&s2);

  printf("  kids forked     : 4\n");
  printf("  pages shared    : 50 per kid\n");
  printf("  cow faults      : %d (should be 0, no writes)\n\n",
    (int)(s2.cow_faults - s1.cow_faults));

  // ---- test 3: cow fork, one kid writes ----
  // one child writes to 25 pages
  // should tigger exactly 25 cow faults (one per page write)

  printf("test 3: cow fork write 25 pages\n");
  line();

  vmstat(&s1);
  uint t1 = uptime();

  pid = fork();
  if(pid == 0){
    // write to 25 pages, each write triggers a cow fault
    for(i = 0; i < 25; i++)
      cowmem[i * 4096] = 'w';
    exit(0);
  }
  wait(0);

  uint t2 = uptime();
  vmstat(&s2);

  printf("  pages written   : 25\n");
  printf("  cow faults      : %d (should be ~25)\n",
    (int)(s2.cow_faults - s1.cow_faults));
  printf("  time taken      : %d ticks\n\n", (int)(t2 - t1));

  // ---- totals ----
  printf("== totals since boot ==\n");
  line();
  vmstat(&s2);
  printf("  total cow faults   : %d\n", (int)s2.cow_faults);
  printf("  total demand faults: %d\n", (int)s2.demand_faults);
  printf("  free pages         : %d\n", (int)s2.free_pages);

  exit(0);
}
