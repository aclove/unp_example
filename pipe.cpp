#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>

#ifndef MAX_PATH
#define MAX_PATH 256
#endif

#ifndef BUFSIZE
#define BUFSIZE 1024
#endif

void server(int read_fd, int write_fd)
{
    char path[MAX_PATH];

    char file_content[BUFSIZE];

    while (true)
    {
        int r_n = read(read_fd, path, MAX_PATH);

        if (strcmp(path, "quit") == 0)
        {
            break;
        }

        if (r_n == -1)
        {
            fprintf(stderr, "[%s] read path fail\n", __FUNCTION__);
            continue;
        }
        else
        {
            printf ("[%s] read path: %s\n", __FUNCTION__, path);
        }



        int file_fd = open(path, O_RDONLY, 0);

        if (-1 == file_fd)
        {
            fprintf(stderr, "[%s] open path fail\n", __FUNCTION__);
            continue;
        }
        else
        {
            printf("%s", path);
        }

        int nbytes = read(file_fd, file_content, BUFSIZE);

        if (-1 == nbytes)
        {
            fprintf(stderr, "[%s] read fail\n", __FUNCTION__);
            continue;
        }

        write(write_fd, file_content, nbytes);
    }

}

void client(int read_fd, int write_fd)
{
    char path[MAX_PATH];

    char file_content[BUFSIZE];

    while (fgets(path, MAX_PATH, stdin))
    {
        int len = strlen(path);
        if (path[len-1] == '\n')
        {
            path[len-1] = '\0';
        }

        if (strcmp(path, "quit") == 0)
        {
            write(write_fd, "quit", strlen("quit")+1);
            break;
        }

        write(write_fd, path, strlen(path)+1);

        int nbytes = read(read_fd, file_content, BUFSIZE-1);

        if (-1 == nbytes)
        {
            fprintf(stderr, "[%s] read fail\n", __FUNCTION__);
            continue;
        }
        file_content[nbytes] = '\0';
        printf ("[%s] file_content: %s", __FUNCTION__, file_content);
    }
}

int main()
{
    int fd1[2], fd2[2];
    pipe(fd1);
    pipe(fd2);

    pid_t pid = fork();
    if (pid < 0)
    {
        exit(1);
    }
    else if (pid > 0)
    {
        // parent
        close(fd1[0]);
        close(fd2[1]);
        server(fd2[0], fd1[1]);
    }
    else
    {
        // child
        close(fd1[1]);
        close(fd2[0]);
        client(fd1[0], fd2[1]);
        exit(0);
    }

    waitpid(pid, NULL, 0);

    return 0;
}
