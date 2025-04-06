#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "../include/helper.hh"

int main()
{
    { // fork(file descriptor)
        print_header("fork-file descriptor");
        // I. write
        int fd = open("example.txt", O_WRONLY | O_CREAT, 0644);
        if (fd == -1)
        {
            perror("open");
            return 1;
        }

        pid_t pid = fork();
        if (pid == 0)
        {
            // 子进程
            write(fd, "Child\n", 6);
            INFO_STREAM("child process, fd: " << fd);
        }
        else if (pid > 0)
        {
            // 父进程
            write(fd, "Parent\n", 7);
            INFO_STREAM("parent process, fd: " << fd);
        }
        else
        {
            perror("fork");
            return 1;
        }
        close(fd);

        // II. read
        // 竞争失败的进程会卡死在getline。
        // if (pid == 0) {
        //     // 子进程
        //     std::string input;
        //     std::cout << "Child: Enter something: ";
        //     std::getline(std::cin, input);  // 从标准输入读取数据
        //     std::cout << "Child received: " << input << std::endl;
        // } else {
        //     // 父进程
        //     std::string input;
        //     std::cout << "Parent: Enter something: ";
        //     std::getline(std::cin, input);  // 从标准输入读取数据
        //     std::cout << "Parent received: " << input << std::endl;
        // }

        if (pid == 0)
        {
            return 0;
        }
    }
    { // fork & execve
        print_header("fork & execve");
        pid_t pid = fork(); // Create a new process

        if (pid < 0)
        {
            // Fork failed
            perror("fork");
            exit(EXIT_FAILURE);
        }
        else if (pid == 0)
        {
            // Child process
            char *argv[] = {"ls", "-l", NULL}; // Arguments for the execve function
            char *envp[] = {NULL};             // Environment variables (can be NULL)

            // Execute the ls command using execve
            execve("/bin/ls", argv, envp);

            // If execve returns, an error occurred
            perror("execve");
            exit(EXIT_FAILURE);
        }
        else
        {
            // Parent process
            wait(NULL); // Wait for the child process to finish
            INFO_STREAM("Child process completed");
        }
    }
    { // malloc & free
        print_header("malloc & free");
        int *p1 = (int *)malloc(4 * sizeof(int));
        if (p1)
        {
            for (int ind = 0; ind < 4; ++ind)
            {
                p1[ind] = ind * ind;
            }
        }
        print_array(p1, sizeof(int));
        free(p1);
    }
    return 0;
}