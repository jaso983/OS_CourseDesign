#include <iostream>
#include <vector>
#include <algorithm>
#include <iomanip>
#include <climits>
#include <queue>

using namespace std;

struct Process {
    string name;
    int arrivalTime;
    int burstTime;
    int priority;      // 新增：优先级
    int remainingTime;
    int completionTime;
    int turnaroundTime;
    double weightedTurnaroundTime;
    bool isFinished;
    bool inQueue;

    Process(string n, int a, int b, int p = 0) : name(n), arrivalTime(a), burstTime(b), priority(p) {
        remainingTime = b;
        completionTime = 0;
        turnaroundTime = 0;
        weightedTurnaroundTime = 0;
        isFinished = false;
        inQueue = false;
    }
};

bool compareArrivalTime(const Process& a, const Process& b) {
    return a.arrivalTime < b.arrivalTime;
}

// 1. FCFS
void calculateFCFS(vector<Process>& processes) {
    sort(processes.begin(), processes.end(), compareArrivalTime);
    int currentTime = 0;
    for (auto& p : processes) {
        if (currentTime < p.arrivalTime) currentTime = p.arrivalTime;
        currentTime += p.burstTime;
        p.completionTime = currentTime;
        p.turnaroundTime = p.completionTime - p.arrivalTime;
        p.weightedTurnaroundTime = (double)p.turnaroundTime / p.burstTime;
    }
}

// 2. SJF (非抢占)
void calculateSJF(vector<Process>& processes) {
    int n = processes.size();
    int completed = 0, currentTime = 0;
    while (completed < n) {
        int idx = -1, minBurst = INT_MAX;
        for (int i = 0; i < n; i++) {
            if (!processes[i].isFinished && processes[i].arrivalTime <= currentTime) {
                if (processes[i].burstTime < minBurst) {
                    minBurst = processes[i].burstTime;
                    idx = i;
                }
            }
        }
        if (idx != -1) {
            currentTime += processes[idx].burstTime;
            processes[idx].completionTime = currentTime;
            processes[idx].turnaroundTime = processes[idx].completionTime - processes[idx].arrivalTime;
            processes[idx].weightedTurnaroundTime = (double)processes[idx].turnaroundTime / processes[idx].burstTime;
            processes[idx].isFinished = true;
            completed++;
        } else {
            currentTime++;
        }
    }
}

// 3. 优先级调度 (非抢占)
void calculatePriority(vector<Process>& processes) {
    int n = processes.size();
    int completed = 0, currentTime = 0;
    while (completed < n) {
        int idx = -1, minPrio = INT_MAX; // 数字越小优先级越高
        for (int i = 0; i < n; i++) {
            if (!processes[i].isFinished && processes[i].arrivalTime <= currentTime) {
                if (processes[i].priority < minPrio) {
                    minPrio = processes[i].priority;
                    idx = i;
                }
            }
        }
        if (idx != -1) {
            currentTime += processes[idx].burstTime;
            processes[idx].completionTime = currentTime;
            processes[idx].turnaroundTime = processes[idx].completionTime - processes[idx].arrivalTime;
            processes[idx].weightedTurnaroundTime = (double)processes[idx].turnaroundTime / processes[idx].burstTime;
            processes[idx].isFinished = true;
            completed++;
        } else {
            currentTime++;
        }
    }
}

// 4. RR
void calculateRR(vector<Process>& processes, int quantum) {
    sort(processes.begin(), processes.end(), compareArrivalTime);
    queue<int> q;
    int n = processes.size(), completed = 0, currentTime = processes[0].arrivalTime;
    q.push(0); processes[0].inQueue = true;

    while (completed < n) {
        if (q.empty()) {
            for (int i = 0; i < n; i++) {
                if (!processes[i].isFinished) {
                    currentTime = max(currentTime, processes[i].arrivalTime);
                    q.push(i); processes[i].inQueue = true;
                    break;
                }
            }
        }
        int idx = q.front(); q.pop();
        int executeTime = min(processes[idx].remainingTime, quantum);
        processes[idx].remainingTime -= executeTime;
        currentTime += executeTime;

        for (int i = 0; i < n; i++) {
            if (!processes[i].isFinished && processes[i].arrivalTime <= currentTime && !processes[i].inQueue) {
                q.push(i); processes[i].inQueue = true;
            }
        }

        if (processes[idx].remainingTime == 0) {
            processes[idx].completionTime = currentTime;
            processes[idx].turnaroundTime = processes[idx].completionTime - processes[idx].arrivalTime;
            processes[idx].weightedTurnaroundTime = (double)processes[idx].turnaroundTime / processes[idx].burstTime;
            processes[idx].isFinished = true;
            completed++;
        } else {
            q.push(idx);
        }
    }
}

void display(const vector<Process>& processes) {
    cout << "\n进程名\t到达\t服务\t优先\t完成\t周转\t带权周转" << endl;
    for (const auto& p : processes) {
        cout << p.name << "\t" << p.arrivalTime << "\t" << p.burstTime << "\t" << p.priority << "\t"
             << p.completionTime << "\t" << p.turnaroundTime << "\t" << fixed << setprecision(2) << p.weightedTurnaroundTime << endl;
    }
}

int main() {
    int n, choice;
    cout << "请输入进程数量: "; cin >> n;
    vector<Process> processes;
    for (int i = 0; i < n; i++) {
        string name; int a, b, p;
        cout << "输入第" << i+1 << "个进程 [名称 到达时间 服务时间 优先级(选填)]: ";
        cin >> name >> a >> b >> p;
        processes.push_back(Process(name, a, b, p));
    }

    cout << "\n请选择算法: 1.FCFS 2.SJF 3.RR 4.Priority : "; cin >> choice;
    if (choice == 1) calculateFCFS(processes);
    else if (choice == 2) calculateSJF(processes);
    else if (choice == 3) { int q; cout << "输入时间片: "; cin >> q; calculateRR(processes, q); }
    else if (choice == 4) calculatePriority(processes);

    display(processes);
    return 0;
}