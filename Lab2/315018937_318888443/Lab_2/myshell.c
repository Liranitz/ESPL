#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <errno.h>
#include "LineParser.h"
#include <fcntl.h>
#include <signal.h>

#define MAX_INPUT_SIZE 2048

void displayPrompt();
void execute(cmdLine *pCmdLine);
int debug_mode = 0;

int main(int argc, char *argv[])
{
    if(argc > 1 && strcmp(argv[1], "-d")==0)
    {
        debug_mode = 1;
        fprintf(stderr,"Debug mode activated\n");
    }
    char inputBuffer[MAX_INPUT_SIZE];
    while (1)
    {
        displayPrompt();

        if (fgets(inputBuffer, sizeof(inputBuffer), stdin) == NULL)
        {
            break; 
        }

        if (inputBuffer[strlen(inputBuffer) - 1] == '\n')
        {
            inputBuffer[strlen(inputBuffer) - 1] = '\0';
        }

        cmdLine *pCmdLine = parseCmdLines(inputBuffer);

        if (strcmp(pCmdLine->arguments[0], "quit") == 0)
        {
            freeCmdLines(pCmdLine);
            break; 
        }

        if (strcmp(pCmdLine->arguments[0], "cd") == 0)
        {
            if(pCmdLine->argCount > 1)
            {
                if(chdir(pCmdLine->arguments[1]) == -1)
                {
                    if(debug_mode)
                    {
                        fprintf(stderr, "cd: %s: %s\n", pCmdLine->arguments[1], strerror(errno));
                    }
                }
            } 
            else
            {
                if(debug_mode)
                {
                    fprintf(stderr, "cd: missing args\n");
                }
            }

            freeCmdLines(pCmdLine);
            continue;
        }

        execute(pCmdLine);

        freeCmdLines(pCmdLine);
    }

    return 0;
}

void displayPrompt()
{
    char path[PATH_MAX];

    // Get the current working directory
    if (getcwd(path, PATH_MAX) == NULL)
    {
        perror("getcwd");
        exit(EXIT_FAILURE);
    }

    // Display the prompt
    printf("%s> ", path);
}

void execute(cmdLine *pCmdLine)
{
    if(strcmp(pCmdLine->arguments[0], "wakeup") == 0 || strcmp(pCmdLine->arguments[0], "nuke") == 0)
    {
        if(pCmdLine->argCount != 2){
            if(debug_mode)
            {
                fprintf(stderr, "Usage: %s <process_id>\n", pCmdLine->arguments[0]);
            }
            return;
        }

        pid_t target_pid = atoi(pCmdLine->arguments[1]);
        int sig_type = (strcmp(pCmdLine->arguments[0], "wakeup") == 0) ? SIGCONT : SIGKILL;

        if(kill(target_pid, sig_type) == -1)
        {
            if(debug_mode)
            {
                fprintf(stderr, "Failed send signal to pid %d: %s\n", target_pid, strerror(errno));
            }
            return;
        }

        printf("Sent signal to process %d\n",target_pid);

        return;
    }

        // Create a child process
    pid_t pid = fork();

    if (pid == -1)
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) // Child process
    {
        
        if(pCmdLine->inputRedirect != NULL)
        {
            int input_num = open(pCmdLine->inputRedirect, O_RDONLY);
            if(input_num == -1)
            {
                if(debug_mode)
                {
                    fprintf(stderr,"Error in open: %s\n", strerror(errno));
                    exit(EXIT_FAILURE);
                }
            }

            dup2(input_num, STDIN_FILENO);
            close(input_num);
        }

        if(pCmdLine->outputRedirect != NULL)
        {
            int output_num = open(pCmdLine->outputRedirect, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
            if(output_num == -1)
            {
                if(debug_mode)
                {
                    fprintf(stderr,"Error in open: %s\n", strerror(errno));
                    exit(EXIT_FAILURE);
                }
            }

            dup2(output_num, STDOUT_FILENO);
            close(output_num);
        }

        // execv - requires the full path of the exec that we would like to run
        // here we can give him only ths "ls" , without the speceific path

        if (execvp(pCmdLine->arguments[0], pCmdLine->arguments) == -1) 
        {
            if(debug_mode)
            {
                fprintf(stderr, "Error executing cmd : %s\n", strerror(errno));
            }
            exit(EXIT_FAILURE);
        }
    }
    else // Parent process
    {
        if(pCmdLine->blocking)
        {
            waitpid(pid, NULL, 0);
        }

        if(debug_mode)
        {
            printf("Child process run at backgound with pid of : %d\n",pid);
        }
    }
}

