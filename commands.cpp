//
// Created by jonas on 12.03.16.
//

#include <unistd.h>
#include <string.h>
#include "commands.h"

int update(int argc, vector<string> argv, string from)
{

  FILE *command = popen("git --work-tree=/home/jonas/Develop/Chatty --git-dir=/home/jonas/Develop/Chatty/.git pull", "r");

  while(!feof(command))
    ;

  pclose(command);

  unlink("chatty");

  command = popen("cmake .. && make", "r");

  while(!feof(command))
    ;

  pclose(command);

  char path[256];
  char dest[256];
  pid_t pid = getpid();
  sprintf(path, "/proc/%d/exe", pid);
  memset(dest, 0, sizeof *dest);
  if (readlink(path, dest, 256) == -1)
    perror("readlink");
  char* args[2];
  args[0] = dest;
  args[1] = NULL;

  if( execv(dest, args) == -1)
  {
    fprintf(stderr, "Failed to reexecute %s %d %s\n", dest, errno, strerror(errno));
  }return 0;
  exit(1);
}