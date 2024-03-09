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
#define MAX_HISTORY_SIZE 20
#define TERMINATED  -1
#define RUNNING 1
#define SUSPENDED 0

typedef struct process {
    cmdLine* cmd;
    pid_t pid;
    int status;
    struct process *next;
} process;

void displayHistory();
void displayPrompt();
void execute(cmdLine *pCmdLine);
void addProcess(process** process_list, cmdLine* cmd, pid_t pid);
void printProcessList(process** process_list);
void freeProcessList(process* process_list);
void updateProcessList(process **process_list);
void updateProcessStatus(process* process_list, int pid, int status);
void suspendProcess(process** process_list, int pid);
void nukeProcess(process** process_list, int pid);
void wakeupProcess(process** process_list, int pid);
void addToHistory(char* command);
char* history[MAX_HISTORY_SIZE];  // Global array to store command history
int history_count = 0;
int history_oldest = 0;  // Index of the oldest command in history

int debug_mode = 0;
process* process_list = NULL;

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
        else if (strcmp(pCmdLine->arguments[0], "history") == 0)
        {
            displayHistory();
            continue;
        }
        else if (strcmp(pCmdLine->arguments[0], "procs") == 0) {
            printProcessList(&process_list);
            freeCmdLines(pCmdLine);
            continue;
        }
        
        else if (strcmp(inputBuffer, "!!") == 0) {
            if (history_count > 0) {
                strcpy(inputBuffer, history[(history_oldest + history_count - 1) % MAX_HISTORY_SIZE]);
                pCmdLine = parseCmdLines(inputBuffer);
                printf("%s\n", inputBuffer);
            } else {
                printf("No commands in history.\n");
                continue;
            }
        } 
        else if (inputBuffer[0] == '!') {
            int n = atoi(inputBuffer + 1);
            if (n > 0 && n <= history_count) {
                strcpy(inputBuffer, history[(history_oldest + n - 1) % MAX_HISTORY_SIZE]);
                pCmdLine = parseCmdLines(inputBuffer);
                printf("%s\n", inputBuffer);  // Echo the command
            } else {
                printf("Invalid history index.\n");
                continue;
            }
        }
        addToHistory(inputBuffer);
        execute(pCmdLine);
        freeCmdLines(pCmdLine);
    }

    freeProcessList(process_list);

    return 0;
}


void displayPrompt()
{
    char path[PATH_MAX];

    if (getcwd(path, PATH_MAX) == NULL)
    {
        perror("getcwd");
        exit(EXIT_FAILURE);
    }

    printf("%s> ", path);
}

void addToHistory(char* command) {
    if (history_count == MAX_HISTORY_SIZE) {
        free(history[0]);
        for (int i = 0; i < MAX_HISTORY_SIZE - 1; i++) {
            history[i] = history[i + 1];
        }
        history_count--;
    }
    history[history_count++] = strdup(command);
}

void execute(cmdLine *pCmdLine) 
{
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
        return;
    }
            // in case of  still doesnt updare move to upper place
    else if(strcmp(pCmdLine->arguments[0], "wakeup") == 0 || strcmp(pCmdLine->arguments[0], "nuke") == 0 || strcmp(pCmdLine->arguments[0], "suspend") == 0)
    {
        if(pCmdLine->argCount != 2){
            if(debug_mode)
            {
                fprintf(stderr, "Usage: %s <process_id>\n", pCmdLine->arguments[0]);
            }
            return;
        }

        pid_t target_pid = atoi(pCmdLine->arguments[1]);
        printf("Got target of : %d", target_pid);
        int sig_type;

        if(strcmp(pCmdLine->arguments[0], "wakeup") == 0)
        {
            sig_type = SIGCONT;
        }
        else if(strcmp(pCmdLine->arguments[0], "nuke") == 0)
        {
            sig_type = SIGINT;
        }
        
        else
        {
            sig_type = SIGTSTP;
        }
        
        if(kill(target_pid, sig_type) == -1)
        {
            if(debug_mode)
            {
                fprintf(stderr, "Failed send signal to pid %d: %s\n", target_pid, strerror(errno));
            }
            fprintf(stderr, "Failed send signal to pid %d: %s\n", target_pid, strerror(errno));
            return;
        }
        
        printf("Sent signal to process %d\n",target_pid);

        return;
    }
    
    else 
    {
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

            if (pCmdLine->next) { // If there is a pipe
                int pipefd[2];
                if (pipe(pipefd) == -1) {
                    perror("pipe");
                    exit(EXIT_FAILURE);
                }
                pid_t child1, child2;
                child1 = fork();
                if (child1 == -1) {
                    perror("fork");
                    exit(EXIT_FAILURE);
                } 
                if (child1 == 0) { // Child 1
                    close(STDOUT_FILENO);
                    dup2(pipefd[1],STDOUT_FILENO);
                    close(pipefd[1]);
                    execvp(pCmdLine -> arguments[0] , pCmdLine -> arguments);
                    perror(strerror(errno));
                    exit(EXIT_SUCCESS); 
                }
                else {
                close(pipefd[1]);
                child2 = fork();
                if (child2 == -1) {
                    perror("fork");
                    exit(EXIT_FAILURE);
                } 
                if (child2 == 0) { // Child 2
                    close(STDIN_FILENO);
                    dup2(pipefd[0],STDIN_FILENO);
                    close(pipefd[0]);
                    execvp(pCmdLine -> next -> arguments[0] , pCmdLine -> next -> arguments);
                    exit(EXIT_SUCCESS);
                }
                else {            
                close(pipefd[0]);
                waitpid(child1, NULL, 0);
                waitpid(child2, NULL, 0);
                }
            }
        }

            else if (execvp(pCmdLine->arguments[0], pCmdLine->arguments) == -1) 
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
                addProcess(&process_list, pCmdLine, pid);
                waitpid(pid, NULL, 0);
            }
            else 
            {
                addProcess(&process_list, pCmdLine, pid);
            }
            if(debug_mode)
            {
                printf("Child process run at backgound with pid of : %d\n",pid);
            }
        }
    }
}

void addProcess(process** process_list, cmdLine* cmd, pid_t pid) {
    process* new_process = malloc(sizeof(process));
    if(!new_process)
    {
        perror("Malloc problem");
        exit(EXIT_FAILURE);
    }

    new_process->cmd = malloc(sizeof(cmdLine));
    if (!new_process->cmd) {
        perror("Malloc problem");
        exit(EXIT_FAILURE);
    }
    new_process->cmd->argCount = cmd->argCount;
    if(cmd->inputRedirect){
    new_process->cmd->inputRedirect = strdup(cmd->inputRedirect);
    }
    if(cmd->outputRedirect){
    new_process->cmd->outputRedirect = strdup(cmd->outputRedirect);
    }
    new_process->cmd->blocking = cmd->blocking;
    for (int i = 0; i < cmd->argCount; i++) {
        ((char**)new_process->cmd->arguments)[i] = strdup(cmd->arguments[i]);
    }

    new_process->pid = pid;
    new_process->status = RUNNING;
    new_process->next = *process_list;
    *process_list = new_process;
}


void printProcessList(process** process_list) {
    printf("print list \n");
    updateProcessList(process_list);
    printf("PID\tCommand\tSTATUS\n");
    process* curr = *process_list;
    while (curr != NULL) {
        printf("%d\t%s\t\t", curr->pid, curr->cmd->arguments[0]);
        switch(curr->status) {
            case TERMINATED:
                printf("Terminated\n");
                break;
            case RUNNING:
                printf("Running\n");
                break;
            case SUSPENDED:
                printf("Suspended\n");
                break;
            default:
                printf("Unknown\n");
        }
        curr = curr->next;
    }
}


void freeProcessList(process* process_list) {
    process* curr = process_list;
    while (curr != NULL) {
        process* temp = curr;
        curr = curr->next;
        freeCmdLines(temp->cmd);
        free(temp);
    }
}

void updateProcessList(process **process_list) {
    printf("Update list \n");
    process* curr = *process_list;
    while (curr != NULL) {
        int status;
        pid_t result = waitpid(curr->pid, &status, WUNTRACED | WNOHANG| WCONTINUED);
        if (result == -1) {
            updateProcessStatus(curr, curr->pid, TERMINATED);
        } else if (result > 0) {
            if (WIFSTOPPED(status)) {
                updateProcessStatus(curr, curr->pid, SUSPENDED);}
            else if (WIFEXITED(status) || WIFSIGNALED(status)) {
                updateProcessStatus(curr, curr->pid, TERMINATED);}
            else
            {
                updateProcessStatus(curr, curr->pid, RUNNING);
            }
            
        }
        
        curr = curr->next;
    }
}

void updateProcessStatus(process* process_list, int pid, int status) {
    process* curr = process_list;
    while (curr != NULL) {
        if (curr->pid == pid) {
            curr->status = status;
            break;
        }
        curr = curr->next;
    }
}

void displayHistory() {
    printf("Command History:\n");
    for (int i = 0; i < history_count; i++) {
        printf("%d: %s\n", i + 1, history[(history_oldest + i) % MAX_HISTORY_SIZE]);
    }
}