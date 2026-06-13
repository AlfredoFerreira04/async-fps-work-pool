
#include <functional>

class Task{
    int priority;
    std::function<void()> task;

    public:
        Task() = delete;

        Task(int priority, std::function<void()> task)
            : priority(priority), task(task){}

        ~Task(){
            // TODO: HANDLE WHAT HAPPENS IF A DESTRUCTOR IS CALLED WITH A NON-EXECUTED TASK?
        }
        
        // runs associated task
        void operator()(){
            task();
        }

};