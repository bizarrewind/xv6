#include "kernel/types.h"
#include "kernel/memlayout.h"
#include "user/user.h"

//
// Tests for copy-on-write fork.
//

// allocate a bunch of pages and verify child write doesn't affect parent
void
simpletest()
{
  uint64 sz = 4096 * 4;
  char *p = sbrk(sz);
  if (p == (char *)-1) {
    printf("simpletest: sbrk failed\n");
    exit(1);
  }

  for (int i = 0; i < sz; i++)
    p[i] = 'A';

  int pid = fork();
  if (pid < 0) {
    printf("simpletest: fork failed\n");
    exit(1);
  }

  if (pid == 0) {
    for (int i = 0; i < sz; i++) {
      if (p[i] != 'A') {
        printf("simpletest: child saw corrupted data\n");
        exit(1);
      }
      p[i] = 'B';
    }
    for (int i = 0; i < sz; i++) {
      if (p[i] != 'B') {
        printf("simpletest: child self-write failed\n");
        exit(1);
      }
    }
    exit(0);
  }

  int status;
  wait(&status);
  if (status != 0) {
    printf("simpletest: child failed\n");
    exit(1);
  }

  for (int i = 0; i < sz; i++) {
    if (p[i] != 'A') {
      printf("simpletest: parent memory was corrupted by child write\n");
      exit(1);
    }
  }

  sbrk(-sz);
  printf("simpletest: OK\n");
}

// check 3 processes sharing same page
void
threetest()
{
  uint64 sz = 4096 * 4;
  char *p = sbrk(sz);
  if (p == (char *)-1) {
    printf("threetest: sbrk failed\n");
    exit(1);
  }

  for (int i = 0; i < sz; i++)
    p[i] = 'X';

  int pid1 = fork();
  if (pid1 < 0) {
    printf("threetest: fork 1 failed\n");
    exit(1);
  }

  if (pid1 == 0) {
    for (int i = 0; i < sz; i++)
      p[i] = 'Y';
    for (int i = 0; i < sz; i++) {
      if (p[i] != 'Y') {
        printf("threetest: child 1 failed\n");
        exit(1);
      }
    }
    exit(0);
  }

  int pid2 = fork();
  if (pid2 < 0) {
    printf("threetest: fork 2 failed\n");
    exit(1);
  }

  if (pid2 == 0) {
    for (int i = 0; i < sz; i++)
      p[i] = 'Z';
    for (int i = 0; i < sz; i++) {
      if (p[i] != 'Z') {
        printf("threetest: child 2 failed\n");
        exit(1);
      }
    }
    exit(0);
  }

  int s1, s2;
  wait(&s1);
  wait(&s2);
  if (s1 != 0 || s2 != 0) {
    printf("threetest: children failed\n");
    exit(1);
  }

  for (int i = 0; i < sz; i++) {
    if (p[i] != 'X') {
      printf("threetest: parent corrupted\n");
      exit(1);
    }
  }

  sbrk(-sz);
  printf("threetest: OK\n");
}

// test pipe read into cow buffer
char buf[4096];

void
filetest()
{
  for (int i = 0; i < sizeof(buf); i++)
    buf[i] = 'C';

  for (int it = 0; it < 4; it++) {
    int p[2];
    if (pipe(p) < 0) {
      printf("filetest: pipe failed\n");
      exit(1);
    }
    if (write(p[1], "hello", 5) != 5) {
      printf("filetest: write failed\n");
      exit(1);
    }

    int pid = fork();
    if (pid < 0) {
      printf("filetest: fork failed\n");
      exit(1);
    }

    if (pid == 0) {
      close(p[1]);
      char cbuf[16];
      memset(cbuf, 0, sizeof(cbuf));
      if (read(p[0], buf, 5) != 5) {
        printf("filetest: read failed\n");
        exit(1);
      }
      if (memcmp(buf, "hello", 5) != 0) {
        printf("filetest: child got wrong data\n");
        exit(1);
      }
      close(p[0]);
      exit(0);
    }

    close(p[0]);
    close(p[1]);

    int status;
    wait(&status);
    if (status != 0) {
      printf("filetest: child failed\n");
      exit(1);
    }

    // parent's buf should still be 'C'
    if (buf[0] != 'C' || buf[1] != 'C' || buf[2] != 'C' || buf[3] != 'C' || buf[4] != 'C') {
      printf("filetest: parent buf corrupted\n");
      exit(1);
    }
  }

  printf("filetest: OK\n");
}

int
main(int argc, char *argv[])
{
  printf("cowtest: start\n");
  simpletest();
  threetest();
  filetest();
  printf("ALL COW TESTS PASSED\n");
  exit(0);
}
