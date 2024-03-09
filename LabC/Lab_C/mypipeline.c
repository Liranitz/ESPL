#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    int pipefd[2];
    pid_t child1, child2;

    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    // Fork first child (child1)
    child1 = fork();
    if (child1 == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (child1 == 0) {
        close(STDOUT_FILENO);

        // Duplicate the write-end of the pipe to standard output
        dup(pipefd[1]);
        close(pipefd[1]);

        // Execute "ls -l"
        char *ls_args[] = {"ls", "-l", NULL};
        execvp(ls_args[0], ls_args);
        perror("execvp ls");
        exit(EXIT_FAILURE);
    }

    close(pipefd[1]);

    child2 = fork();
    if (child2 == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (child2 == 0) {
        close(STDIN_FILENO);

        // Duplicate the read-end of the pipe to standard input
        dup(pipefd[0]);
        close(pipefd[0]);

        // Execute "tail -n 2"
        char *tail_args[] = {"tail", "-n", "2", NULL};
        execvp(tail_args[0], tail_args);
        perror("execvp tail");
        exit(EXIT_FAILURE);
    }

    close(pipefd[0]);
    waitpid(child1, NULL, 0);
    waitpid(child2, NULL, 0);

    return 0;
}