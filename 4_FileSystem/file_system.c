#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BLOCK_SIZE 512
#define MAX_BLOCKS 1024
#define MAX_FILES 32
#define FILENAME_LEN 20

typedef struct {
    char name[FILENAME_LEN];
    int start_block;
    int size;
    int is_used;
} FileEntry;

char disk[MAX_BLOCKS][BLOCK_SIZE];
int fat[MAX_BLOCKS];
FileEntry directory[MAX_FILES];

void init_fs() {
    for (int i = 0; i < MAX_BLOCKS; i++) fat[i] = 0;
    for (int i = 0; i < MAX_FILES; i++) directory[i].is_used = 0;
    printf("磁盘初始化完成。总容量: %d KB, 块大小: %d B, 总块数: %d\n",
           (MAX_BLOCKS * BLOCK_SIZE) / 1024, BLOCK_SIZE, MAX_BLOCKS);
}

int find_free_block() {
    for (int i = 1; i < MAX_BLOCKS; i++) {
        if (fat[i] == 0) return i;
    }
    return -1;
}

// 统计空闲块数
int count_free_blocks() {
    int count = 0;
    for (int i = 0; i < MAX_BLOCKS; i++) {
        if (fat[i] == 0) count++;
    }
    return count;
}

// 统计已用目录项
int count_used_files() {
    int count = 0;
    for (int i = 0; i < MAX_FILES; i++) {
        if (directory[i].is_used) count++;
    }
    return count;
}

// 查找文件目录项索引，找不到返回 -1
int find_file(const char *name) {
    for (int i = 0; i < MAX_FILES; i++) {
        if (directory[i].is_used && strcmp(directory[i].name, name) == 0)
            return i;
    }
    return -1;
}

// 查找空闲目录项索引
int find_free_dir() {
    for (int i = 0; i < MAX_FILES; i++) {
        if (!directory[i].is_used) return i;
    }
    return -1;
}

// 释放文件的 FAT 链
void free_fat_chain(int start_block) {
    int curr = start_block;
    while (curr != -1) {
        int next = fat[curr];
        fat[curr] = 0;
        curr = next;
    }
}

void create_file(const char *name, const char *content) {
    if (find_file(name) != -1) {
        printf("错误：文件 %s 已存在。\n", name);
        return;
    }

    int dir_idx = find_free_dir();
    if (dir_idx == -1) {
        printf("错误：目录已满（最多 %d 个文件）。\n", MAX_FILES);
        return;
    }

    int len = strlen(content);
    int num_blocks = (len == 0) ? 0 : ((len + BLOCK_SIZE - 1) / BLOCK_SIZE);

    // 预检查空间是否足够
    if (num_blocks > count_free_blocks()) {
        printf("错误：磁盘空间不足（需要 %d 块，可用 %d 块）。\n",
               num_blocks, count_free_blocks());
        return;
    }

    int first_block = -1;
    int prev_block = -1;

    for (int i = 0; i < num_blocks; i++) {
        int curr = find_free_block();
        if (curr == -1) {
            // 理论不会到这里（已预检查），但保留安全处理
            free_fat_chain(first_block);
            printf("错误：分配失败，磁盘空间不足。\n");
            return;
        }

        if (i == 0) first_block = curr;

        if (prev_block != -1) fat[prev_block] = curr;
        fat[curr] = -1;

        int bytes_to_copy = (len > BLOCK_SIZE) ? BLOCK_SIZE : len;
        memcpy(disk[curr], content + (i * BLOCK_SIZE), bytes_to_copy);
        len -= bytes_to_copy;

        prev_block = curr;
    }

    strcpy(directory[dir_idx].name, name);
    directory[dir_idx].start_block = first_block;
    directory[dir_idx].size = strlen(content);
    directory[dir_idx].is_used = 1;

    printf("文件 '%s' 创建成功，大小 %d B，占用 %d 个块。\n",
           name, directory[dir_idx].size, num_blocks);
}

void read_file(const char *name) {
    int idx = find_file(name);
    if (idx == -1) {
        printf("错误：文件 %s 未找到。\n", name);
        return;
    }

    printf("===== 读取文件 [%s]（大小: %d B）=====\n", directory[idx].name, directory[idx].size);

    if (directory[idx].size == 0) {
        printf("（空文件）\n");
        return;
    }

    int curr = directory[idx].start_block;
    int remaining = directory[idx].size;

    while (curr != -1 && remaining > 0) {
        int read_size = (remaining > BLOCK_SIZE) ? BLOCK_SIZE : remaining;
        for (int j = 0; j < read_size; j++) {
            putchar(disk[curr][j]);
        }
        remaining -= read_size;
        curr = fat[curr];
    }
    printf("\n");
}

void write_file(const char *name, const char *content) {
    int idx = find_file(name);
    if (idx == -1) {
        printf("错误：文件 %s 不存在。\n", name);
        return;
    }

    // 释放旧 FAT 链
    free_fat_chain(directory[idx].start_block);

    int len = strlen(content);
    int num_blocks = (len == 0) ? 0 : ((len + BLOCK_SIZE - 1) / BLOCK_SIZE);

    if (num_blocks > count_free_blocks()) {
        printf("错误：磁盘空间不足（需要 %d 块，可用 %d 块）。\n",
               num_blocks, count_free_blocks());
        // 文件内容已被清除，标记为空文件
        directory[idx].start_block = -1;
        directory[idx].size = 0;
        return;
    }

    int first_block = -1;
    int prev_block = -1;

    for (int i = 0; i < num_blocks; i++) {
        int curr = find_free_block();
        if (curr == -1) {
            free_fat_chain(first_block);
            directory[idx].start_block = -1;
            directory[idx].size = 0;
            printf("错误：写入失败，磁盘空间不足。\n");
            return;
        }

        if (i == 0) first_block = curr;
        if (prev_block != -1) fat[prev_block] = curr;
        fat[curr] = -1;

        int bytes_to_copy = (len > BLOCK_SIZE) ? BLOCK_SIZE : len;
        memcpy(disk[curr], content + (i * BLOCK_SIZE), bytes_to_copy);
        len -= bytes_to_copy;

        prev_block = curr;
    }

    directory[idx].start_block = first_block;
    directory[idx].size = strlen(content);

    printf("文件 '%s' 写入成功，新大小 %d B，占用 %d 个块。\n",
           name, directory[idx].size, num_blocks);
}

void delete_file(const char *name) {
    int idx = find_file(name);
    if (idx == -1) {
        printf("错误：文件 %s 未找到。\n", name);
        return;
    }

    free_fat_chain(directory[idx].start_block);
    directory[idx].is_used = 0;
    printf("文件 '%s' 已删除，空间已回收。\n", name);
}

void list_files() {
    printf("\n%-4s %-20s %-10s %-10s\n", "编号", "文件名", "大小(B)", "起始块");
    printf("--------------------------------------------------\n");
    int count = 0;
    for (int i = 0; i < MAX_FILES; i++) {
        if (directory[i].is_used) {
            printf("%-4d %-20s %-10d %-10d\n",
                   count + 1, directory[i].name, directory[i].size,
                   directory[i].start_block);
            count++;
        }
    }
    if (count == 0) printf("（目录为空）\n");
    printf("--------------------------------------------------\n");
    printf("共 %d 个文件\n", count);
}

void show_disk_status() {
    int free_blocks = count_free_blocks();
    int used_blocks = MAX_BLOCKS - free_blocks;
    int used_files = count_used_files();

    printf("\n========== 磁盘状态 ==========\n");
    printf("总块数:       %d\n", MAX_BLOCKS);
    printf("已用块数:     %d (%.1f%%)\n", used_blocks,
           100.0 * used_blocks / MAX_BLOCKS);
    printf("空闲块数:     %d (%.1f%%)\n", free_blocks,
           100.0 * free_blocks / MAX_BLOCKS);
    printf("总目录项:     %d\n", MAX_FILES);
    printf("已用目录项:   %d\n", used_files);
    printf("空闲目录项:   %d\n", MAX_FILES - used_files);
    printf("===============================\n");
}

void print_help() {
    printf("\n可用命令:\n");
    printf("  create <文件名> <内容>  - 创建文件并写入内容\n");
    printf("  read   <文件名>         - 读取文件内容\n");
    printf("  write  <文件名> <内容>  - 覆盖写入已有文件\n");
    printf("  delete <文件名>         - 删除文件\n");
    printf("  list                    - 列出所有文件\n");
    printf("  status                  - 显示磁盘状态\n");
    printf("  help                    - 显示此帮助\n");
    printf("  exit                    - 退出\n\n");
}

int main() {
    char cmd[32];
    char arg1[FILENAME_LEN];
    char arg2[BLOCK_SIZE * 10]; // 文件内容缓冲区

    init_fs();
    print_help();

    while (1) {
        printf("\nfs> ");
        fflush(stdout);

        if (scanf("%31s", cmd) != 1) break;

        if (strcmp(cmd, "exit") == 0) {
            printf("再见。\n");
            break;
        } else if (strcmp(cmd, "create") == 0) {
            if (scanf("%19s", arg1) != 1) {
                printf("用法: create <文件名> <内容>\n");
                while (getchar() != '\n');
                continue;
            }
            getchar(); // 跳过空格
            fgets(arg2, sizeof(arg2), stdin);
            arg2[strcspn(arg2, "\n")] = '\0'; // 去换行符

            if (strlen(arg2) == 0) {
                printf("用法: create <文件名> <内容>\n");
                continue;
            }
            create_file(arg1, arg2);

        } else if (strcmp(cmd, "read") == 0) {
            if (scanf("%19s", arg1) != 1) {
                printf("用法: read <文件名>\n");
                while (getchar() != '\n');
                continue;
            }
            read_file(arg1);

        } else if (strcmp(cmd, "write") == 0) {
            if (scanf("%19s", arg1) != 1) {
                printf("用法: write <文件名> <内容>\n");
                while (getchar() != '\n');
                continue;
            }
            getchar();
            fgets(arg2, sizeof(arg2), stdin);
            arg2[strcspn(arg2, "\n")] = '\0';

            if (strlen(arg2) == 0) {
                printf("用法: write <文件名> <内容>\n");
                continue;
            }
            write_file(arg1, arg2);

        } else if (strcmp(cmd, "delete") == 0) {
            if (scanf("%19s", arg1) != 1) {
                printf("用法: delete <文件名>\n");
                while (getchar() != '\n');
                continue;
            }
            delete_file(arg1);

        } else if (strcmp(cmd, "list") == 0) {
            list_files();

        } else if (strcmp(cmd, "status") == 0) {
            show_disk_status();

        } else if (strcmp(cmd, "help") == 0) {
            print_help();

        } else {
            printf("未知命令: %s。输入 help 查看可用命令。\n", cmd);
            while (getchar() != '\n');
        }
    }

    return 0;
}
