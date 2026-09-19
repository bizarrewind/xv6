
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  int res = greet(5);
  printf("Syscall completed with return code %d\n", res);

  exit(0);
}
