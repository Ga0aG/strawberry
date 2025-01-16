#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "../include/helper.hh"

int main() {
    {// fork & execve
        print_header("fork & execve");
        pid_t pid = fork(); // Create a new process

        if (pid < 0) {
            // Fork failed
            perror("fork");
            exit(EXIT_FAILURE);
        } else if (pid == 0) {
            // Child process
            char *argv[] = { "ls", "-l", NULL }; // Arguments for the execve function
            char *envp[] = { NULL }; // Environment variables (can be NULL)

            // Execute the ls command using execve
            execve("/bin/ls", argv, envp);

            // If execve returns, an error occurred
            perror("execve");
            exit(EXIT_FAILURE);
        } else {
            // Parent process
            wait(NULL); // Wait for the child process to finish
            printf("Child process completed.\n");
        }
    }
    {// malloc & free
        print_header("malloc & free");
        int *p1 = (int*)malloc(4*sizeof(int));
        if (p1)
        {
            for (int ind=0 ; ind < 4; ++ind)
            {
                p1[ind] = ind * ind;
            }
        }
        print_array(p1, sizeof(int));
        free(p1);
    }
    return 0;
}