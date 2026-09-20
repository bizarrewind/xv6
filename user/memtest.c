#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  int pages = 10;

  if (argc > 1) {
    pages = atoi(argv[1]);
  }

  printf("Memory inspection\n");

  uint64 fb = freemem();

  printf("Free memory before : %lu bytes (%lu MB)\n", fb, fb / (1024 * 1024));

  printf("asking os for %d pages of memory ", pages);
  char *mem = sbrk(4096 * pages);

  if (mem == (char *)-1) {
    printf("Allocation Failed\n");
    exit(1);
  }
  uint64 fa = freemem();

  printf("Free memory after: %lu bytes (%lu MB)\n", fa, fa / (1024 * 1024));

  printf("Difference = %lu Bytes %lu pages \n", (fb - fa), (fb - fa) / 4096);
  exit(0);
}