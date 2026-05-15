#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BLOCK_SIZE 512       // 每个数据块 512 字节
#define MAX_BLOCKS 1024      // 模拟磁盘共有 1024 个块 (共 512KB)
#define MAX_FILES 32         // 最多存放 32 个文件
#define FILENAME_LEN 20

// 文件条目（目录项）
typedef struct {
    char name[FILENAME_LEN];
    int start_block;
    int size;                // 文件大小（字节）
    int is_used;             // 标识此目录项是否被占用
} FileEntry;

// 模拟磁盘结构
char disk[MAX_BLOCKS][BLOCK_SIZE]; // 数据区
int fat[MAX_BLOCKS];               // FAT表：0代表空闲，-1代表结束，>0代表下一块索引
FileEntry directory[MAX_FILES];    // 根目录

// 初始化磁盘
void init_fs() {
    for (int i = 0; i < MAX_BLOCKS; i++) fat[i] = 0; // 0 表示空闲
    for (int i = 0; i < MAX_FILES; i++) directory[i].is_used = 0;
    printf("磁盘初始化完成。总容量: %d KB\n", (MAX_BLOCKS * BLOCK_SIZE) / 1024);
}

// 寻找空闲 FAT 块
int find_free_block() {
    for (int i = 1; i < MAX_BLOCKS; i++) { // 0号块预留或不用
        if (fat[i] == 0) return i;
    }
    return -1;
}

// 创建文件
void create_file(char *name, char *content) {
    // 1. 检查文件名是否存在
    for (int i = 0; i < MAX_FILES; i++) {
        if (directory[i].is_used && strcmp(directory[i].name, name) == 0) {
            printf("错误：文件 %s 已存在。\n", name);
            return;
        }
    }

    // 2. 找到空的目录项
    int dir_idx = -1;
    for (int i = 0; i < MAX_FILES; i++) {
        if (!directory[i].is_used) {
            dir_idx = i;
            break;
        }
    }

    if (dir_idx == -1) {
        printf("错误：目录已满。\n");
        return;
    }

    // 3. 写入内容并分配块
    int len = strlen(content);
    int num_blocks_needed = (len / BLOCK_SIZE) + 1;
    
    int prev_block = -1;
    int first_block = -1;

    for (int i = 0; i < num_blocks_needed; i++) {
        int current_block = find_free_block();
        if (current_block == -1) {
            printf("错误：磁盘空间不足。\n");
            return;
        }

        if (i == 0) first_block = current_block;
        
        // 更新 FAT 表
        if (prev_block != -1) fat[prev_block] = current_block;
        fat[current_block] = -1; // 暂时标记为结尾

        // 写入数据到模拟磁盘
        int bytes_to_copy = (len > BLOCK_SIZE) ? BLOCK_SIZE : len;
        memcpy(disk[current_block], content + (i * BLOCK_SIZE), bytes_to_copy);
        len -= bytes_to_copy;
        
        prev_block = current_block;
    }

    // 4. 更新目录
    strcpy(directory[dir_idx].name, name);
    directory[dir_idx].start_block = first_block;
    directory[dir_idx].size = strlen(content);
    directory[dir_idx].is_used = 1;

    printf("文件 '%s' 创建成功，占用 %d 个块。\n", name, num_blocks_needed);
}

// 读取文件
void read_file(char *name) {
    for (int i = 0; i < MAX_FILES; i++) {
        if (directory[i].is_used && strcmp(directory[i].name, name) == 0) {
            printf("读取文件 [%s] 内容: ", name);
            int curr = directory[i].start_block;
            int total_to_read = directory[i].size;

            while (curr != -1) {
                int read_size = (total_to_read > BLOCK_SIZE) ? BLOCK_SIZE : total_to_read;
                for (int j = 0; j < read_size; j++) {
                    putchar(disk[curr][j]);
                }
                total_to_read -= read_size;
                curr = fat[curr];
            }
            printf("\n");
            return;
        }
    }
    printf("错误：文件未找到。\n");
}

// 删除文件
void delete_file(char *name) {
    for (int i = 0; i < MAX_FILES; i++) {
        if (directory[i].is_used && strcmp(directory[i].name, name) == 0) {
            // 释放 FAT 链
            int curr = directory[i].start_block;
            while (curr != -1) {
                int next = fat[curr];
                fat[curr] = 0; // 标记为空闲
                curr = next;
            }
            directory[i].is_used = 0; // 释放目录项
            printf("文件 '%s' 已删除，空间已回收。\n", name);
            return;
        }
    }
    printf("错误：未找到文件。\n");
}

// 列出所有文件
void list_files() {
    printf("\n--- 当前文件系统列表 ---\n");
    printf("%-20s %-10s %-10s\n", "文件名", "大小(Byte)", "起始块号");
    for (int i = 0; i < MAX_FILES; i++) {
        if (directory[i].is_used) {
            printf("%-20s %-10d %-10d\n", directory[i].name, directory[i].size, directory[i].start_block);
        }
    }
    printf("------------------------\n");
}

int main() {
    init_fs();

    create_file("hello.txt", "Hello Operating System! This is a simple file system simulation.");
    create_file("test.c", "int main() { printf(\"Hello World\"); return 0; }");
    
    list_files();

    read_file("hello.txt");

    delete_file("hello.txt");
    
    list_files();

    create_file("new_file.txt", "This file might reuse the old blocks.");
    list_files();

    return 0;
}