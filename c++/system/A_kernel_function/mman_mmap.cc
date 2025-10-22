#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>

// g++ system/kernel/mman_mmap.cc -o build/mman_mmap
// ./build/mman_mmap  /home/zhuojun/workspace/strawberry/README.md

int main(int argc, char *argv[])
{
    if (argc == 0)
    {
        return EXIT_FAILURE;
    }

    const char *filename = argv[1];
    printf("File name: %s", filename);
    int fd = open(filename, O_RDONLY);
    if (fd == -1)
    {
        perror("Error opening file");
        return EXIT_FAILURE;
    }

    struct stat sb;
    fstat(fd, &sb);
    size_t length = sb.st_size;

    char *mapped = static_cast<char *>(mmap(NULL, length, PROT_READ, MAP_SHARED, fd, 0));
    if (mapped == MAP_FAILED)
    {
        perror("Error mapping file");
        close(fd);
        return EXIT_FAILURE;
    }

    // 读取映射区域
    printf("File content: %s", mapped);

    // 清理
    munmap(mapped, length);
    close(fd);
    return EXIT_SUCCESS;
}