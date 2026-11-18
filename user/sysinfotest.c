#include "kernel/types.h"
#include "kernel/sysinfo.h"
#include "user/user.h"

// check freemem decreases and increases
void
testmem(void)
{
  struct sysinfo info;
  if (sysinfo(&info) < 0) {
    printf("sysinfotest: sysinfo failed\n");
    exit(1);
  }
  uint64 m0 = info.freemem;

  int n_pages = 10;
  char *p = sbrk(n_pages * 4096);
  if (p == (char *)-1) {
    printf("sysinfotest: sbrk failed\n");
    exit(1);
  }

  for (int i = 0; i < n_pages; i++) {
    p[i * 4096] = 1;
  }

  if (sysinfo(&info) < 0) {
    printf("sysinfotest: sysinfo failed\n");
    exit(1);
  }
  uint64 m1 = info.freemem;

  if (m1 >= m0 || (m0 - m1) < (uint64)n_pages * 4096) {
    printf("sysinfotest: freemem test failed (m0=%ld, m1=%ld)\n", m0, m1);
    exit(1);
  }

  sbrk(-n_pages * 4096);

  if (sysinfo(&info) < 0) {
    printf("sysinfotest: sysinfo failed\n");
    exit(1);
  }
  uint64 m2 = info.freemem;

  if (m2 <= m1) {
    printf("sysinfotest: freemem restore failed (m1=%ld, m2=%ld)\n", m1, m2);
    exit(1);
  }

  printf("sysinfotest: freemem test passed\n");
}

// check nproc count updates
void
testproc(void)
{
  struct sysinfo info;
  if (sysinfo(&info) < 0) {
    printf("sysinfotest: sysinfo failed\n");
    exit(1);
  }
  uint64 n0 = info.nproc;

  int pid = fork();
  if (pid < 0) {
    printf("sysinfotest: fork failed\n");
    exit(1);
  }

  if (pid == 0) {
    struct sysinfo cinfo;
    if (sysinfo(&cinfo) < 0) {
      printf("sysinfotest: sysinfo in child failed\n");
      exit(1);
    }
    if (cinfo.nproc != n0 + 1) {
      printf("sysinfotest: child nproc expected %ld, got %ld\n", n0 + 1, cinfo.nproc);
      exit(1);
    }
    exit(0);
  }

  int status;
  wait(&status);
  if (status != 0) {
    printf("sysinfotest: child failed\n");
    exit(1);
  }

  if (sysinfo(&info) < 0) {
    printf("sysinfotest: sysinfo after wait failed\n");
    exit(1);
  }
  uint64 n1 = info.nproc;

  if (n1 != n0) {
    printf("sysinfotest: nproc after wait expected %ld, got %ld\n", n0, n1);
    exit(1);
  }

  printf("sysinfotest: nproc test passed\n");
}

void
testbad(void)
{
  if (sysinfo((struct sysinfo *)0xffffffffffffffff) != -1) {
    printf("sysinfotest: bad address should fail\n");
    exit(1);
  }
  printf("sysinfotest: bad address test passed\n");
}

int
main(int argc, char *argv[])
{
  printf("sysinfotest: start\n");
  testmem();
  testproc();
  testbad();
  printf("sysinfotest: OK\n");
  exit(0);
}
