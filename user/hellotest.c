#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  printf(" starting user program");
  //call our new systall pass arg
  int result = hello(32);
  printf("Syscall returned with status: %d\n", result);
  exit(0);
}
