#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define MESSAGE "hello"
#define BUFFER_SIZE 128

int main() {
    int pipefd[2];
    pid_t pid;
    char buffer[BUFFER_SIZE];

    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    // Create a child process
    pid = fork();

    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) { // Child process
        close(pipefd[0]);

        if (write(pipefd[1], MESSAGE, strlen(MESSAGE)) == -1) {
            perror("write");
            exit(EXIT_FAILURE);
        }


        exit(EXIT_SUCCESS);
    } else { // Parent process
        // Close the write end of the pipe
        close(pipefd[1]);

        // Read the message from the pipe
        ssize_t bytesRead = read(pipefd[0], buffer, BUFFER_SIZE - 1);

        if (bytesRead == -1) {
            perror("read");
            exit(EXIT_FAILURE);
        }

        buffer[bytesRead] = '\0';

        printf("Parent received message: %s\n", buffer);

        close(pipefd[0]);

        exit(EXIT_SUCCESS);
    }

    return 0;
}