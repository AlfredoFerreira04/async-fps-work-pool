#include "work_pool/work_pool.cpp"
#include "task.cpp"

#include <chrono>
#include <iostream>
#include <thread>

void run_task(int priority, int delay_ms) {
    std::cout << "task " << priority << " started\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
    std::cout << "task " << priority << " finished\n";
}

int main() {
    WorkPool<Task> pool(2);

    auto first = pool.submit_work(run_task, 1, 1, 200);
    auto second = pool.submit_work(run_task, 2, 2, 200);
    auto third = pool.submit_work(run_task, 3, 3, 200);
    auto fourth = pool.submit_work(run_task, 4, 4, 200);
    auto fifth = pool.submit_work(run_task, 5, 5, 200);

    fifth.get();
    fourth.get();
    third.get();
    second.get();
    first.get();
    

    return 0;
}
