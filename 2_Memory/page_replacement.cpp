#include <iostream>
#include <vector>
#include <list>
#include <algorithm>
#include <iomanip>

using namespace std;

void print_step(int page, const vector<int>& frames, bool fault) {
    cout << "访问页面 " << setw(2) << page << " | 内存状态: [";
    for (int i = 0; i < frames.size(); ++i) {
        if (frames[i] == -1) cout << "  ";
        else cout << setw(2) << frames[i];
        if (i < frames.size() - 1) cout << ",";
    }
    cout << "] | " << (fault ? "产生缺页 <--" : "命中") << endl;
}

// FIFO 算法
void do_FIFO(vector<int> pages, int frame_count) {
    vector<int> frames(frame_count, -1);
    int head = 0; // 指向最老进入的页面位置
    int faults = 0;

    cout << "\n--- FIFO 页面置换过程 ---" << endl;
    for (int page : pages) {
        bool found = false;
        for (int f : frames) {
            if (f == page) {
                found = true;
                break;
            }
        }

        bool fault = false;
        if (!found) {
            faults++;
            fault = true;
            frames[head] = page;
            head = (head + 1) % frame_count; // 循环队列替换
        }
        print_step(page, frames, fault);
    }
    cout << "总缺页次数: " << faults << " | 缺页率: " << fixed << setprecision(2) 
         << (double)faults / pages.size() * 100 << "%" << endl;
}

// LRU 算法
void do_LRU(vector<int> pages, int frame_count) {
    vector<int> frames(frame_count, -1);
    vector<int> counter(frame_count, 0); // 记录页面多久没被使用了
    int faults = 0;

    cout << "\n--- LRU 页面置换过程 ---" << endl;
    for (int page : pages) {
        bool found = false;
        int hit_index = -1;

        for (int i = 0; i < frame_count; i++) {
            if (frames[i] == page) {
                found = true;
                hit_index = i;
                break;
            }
        }

        bool fault = false;
        if (found) {
            // 命中，更新计数器：当前命中页面清零，其他增加
            for (int i = 0; i < frame_count; i++) counter[i]++;
            counter[hit_index] = 0;
        } else {
            faults++;
            fault = true;
            // 找空位或者找计数器最大的（最久没用的）
            int replace_idx = 0;
            int max_val = -1;
            for (int i = 0; i < frame_count; i++) {
                if (frames[i] == -1) {
                    replace_idx = i;
                    break;
                }
                if (counter[i] > max_val) {
                    max_val = counter[i];
                    replace_idx = i;
                }
            }
            frames[replace_idx] = page;
            for (int i = 0; i < frame_count; i++) counter[i]++;
            counter[replace_idx] = 0;
        }
        print_step(page, frames, fault);
    }
    cout << "总缺页次数: " << faults << " | 缺页率: " << fixed << setprecision(2) 
         << (double)faults / pages.size() * 100 << "%" << endl;
}

int main() {
    int f_count, p_count;
    cout << "请输入物理块(Frames)数量: ";
    cin >> f_count;
    cout << "请输入访问页面序列长度: ";
    cin >> p_count;
    
    vector<int> pages(p_count);
    cout << "请输入页面序列 (空格分隔): ";
    for (int i = 0; i < p_count; i++) cin >> pages[i];

    do_FIFO(pages, f_count);
    do_LRU(pages, f_count);

    return 0;
}