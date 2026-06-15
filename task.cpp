
#include <functional>

class Task{
    int priority;
    std::function<void()> task;

    public:
        Task() = delete;

        Task(int priority, std::function<void()> task)
            : priority(priority), task(std::move(task)){}
        
        // runs associated task
        void operator()(){
            task();
        }

        int getPriority() const {return this->priority;}

};