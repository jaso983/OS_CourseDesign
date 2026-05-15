#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

#define MAX_LINE 80
#define MAX_ARGS 10

void show_sys_info() {
    char buffer[1024];
    int fd;

    printf("\n--- [Kernel Status from /proc] ---\n");
    
    // 1. 读取内核版本
    fd = open("/proc/version", O_RDONLY);
    if (fd != -1) {
        read(fd, buffer, sizeof(buffer) - 1);
        printf("Kernel Version: %.60s...\n", buffer); // 只打印前60个字符
        close(fd);
    }

    // 2. 读取内存状态
    fd = open("/proc/meminfo", O_RDONLY);
    if (fd != -1) {
        read(fd, buffer, 128); // 读取前128字节
        printf("Memory Info:\n%s", buffer);
        close(fd);
    }

    // 3. 读取系统运行时间
    fd = open("/proc/uptime", O_RDONLY);
    if (fd != -1) {
        read(fd, buffer, 64);
        printf("System Uptime (seconds): %s", buffer);
        close(fd);
    }
    printf("----------------------------------\n\n");
}

void execute_command(char *args[], int background) {
    pid_t pid = fork();

    if (pid < 0) {
        perror("Fork 失败");
        return;
    } else if (pid == 0) {
        // 子进程：执行命令
        if (execvp(args[0], args) < 0) {
            printf("错误：找不到命令 '%s'\n", args[0]);
            exit(1);
        }
    } else {
        // 父进程
        if (!background) {
            waitpid(pid, NULL, 0); // 等待子进程结束
        } else {
            printf("[后台运行] PID: %d\n", pid);
        }
    }
}

int main() {
    char *args[MAX_ARGS]; 
    char input[MAX_LINE];
    int should_run = 1;

    while (should_run) {
        printf("OS-Shell> ");
        fflush(stdout);

        // 读取输入
        if (!fgets(input, MAX_LINE, stdin)) break;
        input[strcspn(input, "\n")] = 0; // 去掉换行符

        // 1. 处理内置命令：exit
        if (strcmp(input, "exit") == 0) {
            should_run = 0;
            continue;
        }

        // 2. 处理内置命令：sysinfo (读取 /proc)
        if (strcmp(input, "sysinfo") == 0) {
            show_sys_info();
            continue;
        }

        // 3. 解析参数
        int i = 0;
        int background = 0;
        char *token = strtok(input, " ");
        while (token != NULL) {
            if (strcmp(token, "&") == 0) {
                background = 1;
            } else {
                args[i++] = token;
            }
            token = strtok(NULL, " ");
        }
        args[i] = NULL;

        if (args[0] == NULL) continue;

        // 4. 执行外部命令
        execute_command(args, background);
    }

    return 0;
}