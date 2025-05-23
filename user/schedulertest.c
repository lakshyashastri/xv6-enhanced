#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fcntl.h"

#define NFORK 10
#define IO 0

int main()
{
  int n, pid;
  int wtime, rtime;
  int twtime = 0, trtime = 0;
  for (n = 0; n < NFORK; n++)
  {
    pid = fork();
    if (pid < 0)
      break;
    if (pid == 0)
    {
#ifndef FCFS
      if (n < IO)
      {
        sleep(200); // IO bound processes
      }
      else
      {
#endif
        for (volatile int i = 0; i < 1000000000; i++)
        {
        } // CPU bound process
#ifndef FCFS
      }
#endif
      printf("\n%d", n);
      exit(0);
    }
    else
    {
#ifdef PBS
      set_priority(60 - IO + n, pid); // Will only matter for PBS, set lower priority for IO bound processes
#endif
    }
  }
  for (; n > 0; n--)
  {
    if (waitx(0, &wtime, &rtime) >= 0) // fixed :)
    {
      trtime += rtime;
      twtime += wtime;
    }
  }
  printf("\nAverage rtime %d,  wtime %d\n", trtime / NFORK, twtime / NFORK);
  exit(0);
}