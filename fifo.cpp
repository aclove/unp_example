#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>

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
            printf("[%s] recv path %s\n", __FUNCTION__, path);
        }

        int nbytes = read(file_fd, file_content, BUFSIZE);

        if (-1 == nbytes)
        {
            fprintf(stderr, "[%s] read fail\n", __FUNCTION__);
            continue;
        }

        fflush(stdout);

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

        printf("[%s] send path: %s\n", __FUNCTION__, path);

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

#define FIFO0 "/tmp/fifo.0"
#define FIFO1 "/tmp/fifo.1"

int main()
{
    if (mkfifo(FIFO0, 0777) < 0 && errno != EEXIST)
    {
        fprintf(stderr, "[%s] mkfifo fifo0 fail\n", __FUNCTION__);
        return 0;
    }

    if (mkfifo(FIFO1, 0777) < 0 && errno != EEXIST)
    {
        fprintf(stderr, "[%s] mkfifo fifo0 fail\n", __FUNCTION__);
        return 0;
    }

    pid_t pid = fork();
    if (pid < 0)
    {
        exit(1);
    }
    else if (pid > 0)
    {
        // parent
        int fd0, fd1;
        fd0 = open(FIFO0, O_RDONLY, 0);
        fd1 = open(FIFO1, O_WRONLY, 0);
        if (fd0 == -1 || fd1 == -1)
        {
            fprintf(stderr, "[%s] fd open fail %d_%d\n", __FUNCTION__, fd0, fd1);
            goto end;
        }
        server(fd0, fd1);
    }
    else
    {
        // child
        int fd0, fd1;
        fd0 = open(FIFO0, O_WRONLY, 0);
        fd1 = open(FIFO1, O_RDONLY, 0);
        if (fd0 == -1 || fd1 == -1)
        {
            fprintf(stderr, "[%s] fd open fail %d_%d\n", __FUNCTION__, fd0, fd1);
        }
        client(fd1, fd0);
        exit(0);
    }

end:
    waitpid(pid, NULL, 0);

    // unlink(FIFO0);
    // unlink(FIFO1);

    return 0;
}
