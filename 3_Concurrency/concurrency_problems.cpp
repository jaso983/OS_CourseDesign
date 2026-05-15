#include <iostream>
#include <pthread.h>
#include <semaphore.h>
#include <vector>
#include <unistd.h>
#include <iomanip>

using namespace std;

// ==================== 1. 生产者-消费者问题 ====================
const int BUFFER_SIZE = 5;
int buffer[BUFFER_SIZE];
int in = 0, out = 0;
sem_t empty_sem, full_sem;
pthread_mutex_t pc_mutex;

void* producer(void* arg) {
    int id = *(int*)arg;
    for (int i = 0; i < 3; i++) { // 每个生产者生产3个
        sleep(1); // 模拟生产耗时
        int item = rand() % 100;
        
        sem_wait(&empty_sem);
        pthread_mutex_lock(&pc_mutex);
        
        buffer[in] = item;
        cout << "[生产者 " << id << "] 生产了: " << item << " 放入位置 " << in << endl;
        in = (in + 1) % BUFFER_SIZE;
        
        pthread_mutex_unlock(&pc_mutex);
        sem_post(&full_sem);
    }
    return nullptr;
}

void* consumer(void* arg) {
    int id = *(int*)arg;
    for (int i = 0; i < 3; i++) { // 每个消费者消费3个
        sleep(2); // 模拟消费耗时
        
        sem_wait(&full_sem);
        pthread_mutex_lock(&pc_mutex);
        
        int item = buffer[out];
        cout << "[消费者 " << id << "] 消费了: " << item << " 从位置 " << out << endl;
        out = (out + 1) % BUFFER_SIZE;
        
        pthread_mutex_unlock(&pc_mutex);
        sem_post(&empty_sem);
    }
    return nullptr;
}

// ==================== 2. 读者-写者问题 (读者优先) ====================
int read_count = 0;
pthread_mutex_t rw_mutex, count_mutex;

void* reader(void* arg) {
    int id = *(int*)arg;
    sleep(rand() % 2);
    
    pthread_mutex_lock(&count_mutex);
    if (read_count == 0) pthread_mutex_lock(&rw_mutex); // 第一个读者进入，锁住写者
    read_count++;
    pthread_mutex_unlock(&count_mutex);

    cout << "  [读者 " << id << "] 正在读取数据..." << endl;
    sleep(1); 

    pthread_mutex_lock(&count_mutex);
    read_count--;
    if (read_count == 0) pthread_mutex_unlock(&rw_mutex); // 最后一个读者离开，释放写者
    pthread_mutex_unlock(&count_mutex);
    
    cout << "  [读者 " << id << "] 读取完毕退出。" << endl;
    return nullptr;
}

void* writer(void* arg) {
    int id = *(int*)arg;
    sleep(rand() % 3);
    
    pthread_mutex_lock(&rw_mutex); // 写者独占
    cout << "  => [写者 " << id << "] 正在写入数据..." << endl;
    sleep(2);
    cout << "  => [写者 " << id << "] 写入完成。" << endl;
    pthread_mutex_unlock(&rw_mutex);
    
    return nullptr;
}

// ==================== 3. 哲学家进餐问题 ====================
pthread_mutex_t chopsticks[5];

void* philosopher(void* arg) {
    int id = *(int*)arg;
    for (int i = 0; i < 2; i++) { // 每个哲学家吃两次
        cout << "    {哲学家 " << id << "} 正在思考..." << endl;
        sleep(1);

        // 死锁避免策略：奇数号先拿左手，偶数号先拿右手
        if (id % 2 == 0) {
            pthread_mutex_lock(&chopsticks[id]);
            pthread_mutex_lock(&chopsticks[(id + 1) % 5]);
        } else {
            pthread_mutex_lock(&chopsticks[(id + 1) % 5]);
            pthread_mutex_lock(&chopsticks[id]);
        }

        cout << "    {哲学家 " << id << "} 正在用餐..." << endl;
        sleep(1);

        pthread_mutex_unlock(&chopsticks[id]);
        pthread_mutex_unlock(&chopsticks[(id + 1) % 5]);
        cout << "    {哲学家 " << id << "} 吃饱了，放下筷子。" << endl;
    }
    return nullptr;
}

// ==================== 主函数：测试菜单 ====================
int main() {
    int choice;
    while (true) {
        cout << "\n========== 进程同步演示 ==========" << endl;
        cout << "1. 生产者-消费者问题" << endl;
        cout << "2. 读者-写者问题 (读者优先)" << endl;
        cout << "3. 哲学家进餐问题" << endl;
        cout << "4. 退出" << endl;
        cout << "选择: ";
        cin >> choice;

        if (choice == 1) {
            sem_init(&empty_sem, 0, BUFFER_SIZE);
            sem_init(&full_sem, 0, 0);
            pthread_mutex_init(&pc_mutex, nullptr);
            
            pthread_t threads[6];
            int ids[6] = {1, 2, 3, 1, 2, 3};
            for(int i=0; i<3; i++) pthread_create(&threads[i], nullptr, producer, &ids[i]);
            for(int i=3; i<6; i++) pthread_create(&threads[i], nullptr, consumer, &ids[i]);
            for(int i=0; i<6; i++) pthread_join(threads[i], nullptr);
            
            sem_destroy(&empty_sem); sem_destroy(&full_sem);
            pthread_mutex_destroy(&pc_mutex);
        } 
        else if (choice == 2) {
            pthread_mutex_init(&rw_mutex, nullptr);
            pthread_mutex_init(&count_mutex, nullptr);
            read_count = 0;
            
            pthread_t r[5], w[2];
            int rids[5]={1,2,3,4,5}, wids[2]={1,2};
            for(int i=0; i<2; i++) pthread_create(&w[i], nullptr, writer, &wids[i]);
            for(int i=0; i<5; i++) pthread_create(&r[i], nullptr, reader, &rids[i]);
            for(int i=0; i<5; i++) pthread_join(r[i], nullptr);
            for(int i=0; i<2; i++) pthread_join(w[i], nullptr);
            
            pthread_mutex_destroy(&rw_mutex); pthread_mutex_destroy(&count_mutex);
        } 
        else if (choice == 3) {
            for(int i=0; i<5; i++) pthread_mutex_init(&chopsticks[i], nullptr);
            pthread_t ph[5];
            int pids[5]={0,1,2,3,4};
            for(int i=0; i<5; i++) pthread_create(&ph[i], nullptr, philosopher, &pids[i]);
            for(int i=0; i<5; i++) pthread_join(ph[i], nullptr);
            for(int i=0; i<5; i++) pthread_mutex_destroy(&chopsticks[i]);
        } 
        else break;
    }
    return 0;
}