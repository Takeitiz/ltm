#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <time.h>

void signalHandler(int signo)
{
    pid_t pid = wait(NULL);
    printf("Child process terminated, pid = %d\n", pid);
}

void format_time(char *format, char *time_str, size_t max_size)
{
    time_t raw_time;
    struct tm *time_info;

    time(&raw_time);
    time_info = localtime(&raw_time);

    if (strcmp(format, "dd/mm/yyyy") == 0)
    {
        strftime(time_str, max_size, "%d/%m/%Y\n", time_info);
    }
    else if (strcmp(format, "dd/mm/yy") == 0)
    {
        strftime(time_str, max_size, "%d/%m/%y\n", time_info);
    }
    else if (strcmp(format, "mm/dd/yyyy") == 0)
    {
        strftime(time_str, max_size, "%m/%d/%Y\n", time_info);
    }
    else if (strcmp(format, "mm/dd/yy") == 0)
    {
        strftime(time_str, max_size, "%m/%d/%y\n", time_info);
    }
    else
    {
        strncpy(time_str, "Invalid format\n", max_size);
    }
}

int main()
{
    // Tao socket cho ket noi
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener == -1)
    {
        perror("socket() failed");
        return 1;
    }

    // Khai bao dia chi server
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(8000);

    // Gan socket voi cau truc dia chi
    if (bind(listener, (struct sockaddr *)&addr, sizeof(addr)))
    {
        perror("bind() failed");
        return 1;
    }

    // Chuyen socket sang trang thai cho ket noi
    if (listen(listener, 5))
    {
        perror("listen() failed");
        return 1;
    }

    signal(SIGCHLD, signalHandler);

    while (1)
    {
        printf("Waiting for new client\n");
        int client = accept(listener, NULL, NULL);
        printf("New client accepted, client = %d\n", client);

        if (fork() == 0)
        {
            while (1)
            {
                close(listener);

                char buf[256];

                int len = recv(client, buf, sizeof(buf), 0);
                if (len <= 0)
                {
                    break;
                }
                buf[len] = '\0';

                char cmd[32], format[32], tmp[65], line[65];
                int n = sscanf(buf, "%s %s %s", cmd, format, tmp);

                if (n != 2)
                {
                    char *msg = "Sai cu phap. Hay nhap lai.\n";
                    send(client, msg, strlen(msg), 0);
                }
                else if (n == 2 && strcmp(cmd, "GET_TIME") == 0)
                {
                    char time_str[100];
                    format_time(format, time_str, sizeof(time_str));
                    send(client, time_str, strlen(time_str), 0);
                }
                else
                {
                    char *msg = "Invalid command or format. Use 'GET_TIME [format]'.\n";
                    send(client, msg, strlen(msg), 0);
                }
            }
            exit(0);
        }

        close(client);
    }
}