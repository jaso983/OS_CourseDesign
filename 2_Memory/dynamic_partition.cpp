#include <iostream>
#include <vector>
#include <list>
#include <iomanip>
#include <string>
#include <algorithm>

using namespace std;

// 内存块结构体
struct Block {
    int start;    // 起始地址
    int size;     // 大小
    bool is_free; // 是否空闲
    string pid;   // 占用进程号

    Block(int s, int sz, bool f, string p = "None") 
        : start(s), size(sz), is_free(f), pid(p) {}
};

class MemoryManager {
private:
    list<Block> memory_map;
    int total_size;

public:
    MemoryManager(int size) : total_size(size) {
        memory_map.push_back(Block(0, size, true));
    }

    // 打印当前内存状态
    void show_status() {
        cout << "\n当前内存分布图 (总容量: " << total_size << " KB):" << endl;
        cout << string(60, '-') << endl;
        cout << "| " << setw(10) << "起始地址" << " | " << setw(10) << "结束地址" 
             << " | " << setw(8) << "大小" << " | " << setw(10) << "状态" << " |" << endl;
        cout << string(60, '-') << endl;
        for (const auto& b : memory_map) {
            cout << "| " << setw(10) << b.start 
                 << " | " << setw(10) << b.start + b.size 
                 << " | " << setw(8) << b.size 
                 << " | " << setw(10) << (b.is_free ? "空闲" : b.pid) << " |" << endl;
        }
        cout << string(60, '-') << endl;
    }

    // 首次适应算法 (First Fit)
    bool allocate_FF(string pid, int req_size) {
        for (auto it = memory_map.begin(); it != memory_map.end(); ++it) {
            if (it->is_free && it->size >= req_size) {
                split_block(it, pid, req_size);
                return true;
            }
        }
        return false;
    }

    // 最佳适应算法 (Best Fit)
    bool allocate_BF(string pid, int req_size) {
        auto best_it = memory_map.end();
        int min_waste = total_size + 1;

        for (auto it = memory_map.begin(); it != memory_map.end(); ++it) {
            if (it->is_free && it->size >= req_size) {
                if (it->size - req_size < min_waste) {
                    min_waste = it->size - req_size;
                    best_it = it;
                }
            }
        }

        if (best_it != memory_map.end()) {
            split_block(best_it, pid, req_size);
            return true;
        }
        return false;
    }

    // 拆分内存块
    void split_block(list<Block>::iterator it, string pid, int req_size) {
        if (it->size > req_size) {
            // 在当前节点后插入一个新的空闲块
            memory_map.insert(next(it), Block(it->start + req_size, it->size - req_size, true));
        }
        it->size = req_size;
        it->is_free = false;
        it->pid = pid;
    }

    // 回收内存并合并
    void deallocate(string pid) {
        bool found = false;
        for (auto& b : memory_map) {
            if (!b.is_free && b.pid == pid) {
                b.is_free = true;
                b.pid = "None";
                found = true;
            }
        }
        if (!found) {
            cout << "未找到进程 " << pid << " 的内存块。" << endl;
            return;
        }
        // 合并相邻空闲块
        for (auto it = memory_map.begin(); it != memory_map.end(); ) {
            auto next_it = next(it);
            if (next_it != memory_map.end() && it->is_free && next_it->is_free) {
                it->size += next_it->size;
                memory_map.erase(next_it);
            } else {
                ++it;
            }
        }
    }
};

int main() {
    int capacity, choice;
    cout << "请输入模拟内存总容量 (KB): ";
    cin >> capacity;
    MemoryManager mm(capacity);

    while (true) {
        cout << "\n--- 内存管理模拟 (1.FF 2.BF 3.回收 4.退出) ---" << endl;
        cout << "选择操作: ";
        cin >> choice;
        if (choice == 4) break;

        string pid;
        int sz;
        if (choice == 1 || choice == 2) {
            cout << "进程ID: "; cin >> pid;
            cout << "申请大小 (KB): "; cin >> sz;
            bool ok = (choice == 1) ? mm.allocate_FF(pid, sz) : mm.allocate_BF(pid, sz);
            if (!ok) cout << "错误：内存不足，分配失败！" << endl;
        } else if (choice == 3) {
            cout << "输入要回收的进程ID: "; cin >> pid;
            mm.deallocate(pid);
        }
        mm.show_status();
    }
    return 0;
}