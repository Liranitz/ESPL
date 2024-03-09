#include "util.h"

#define SYS_WRITE 4
#define STDOUT 1
#define SYS_OPEN 5
#define O_RDWR 2
#define SYS_SEEK 19
#define SEEK_SET 0
#define SHIRA_OFFSET 0x291

extern int system_call();

int main (int argc , char* argv[], char* envp[])
{
  /*Complete the task here*/
  int i, j;

  /*0.A*/
  for(i = 1 ; i < argc ; i++)
  {
    for(j = 0 ; argv[i][j] != '\0' ; j++){
      system_call(SYS_WRITE,STDOUT, &argv[i][j],1);
    }
    char make_new_line = '\n';
    system_call(SYS_WRITE,STDOUT, &make_new_line,1);
  }

  return 0;

}
