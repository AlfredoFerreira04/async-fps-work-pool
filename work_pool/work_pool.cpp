#include "task_tree.cpp"
#include <atomic>
#include <semaphore>
#include <vector>
#include <thread>
#include <mutex>
#include <future>
#include <memory>
#include <functional>
#include <optional>
#include <stdexcept>
#include <climits>

template<typename T>
concept WorkItem = requires (T t, int priority, std::function<void()> fn) {
    t();
    t.getPriority();
    T{priority, fn};
};

template<WorkItem T>
class WorkPool{
    
    int nr_workers;
    std::atomic<bool> stopping{false};

    // data structures
    TaskTree<T> tree;
    std::vector<std::thread> workers;

    // concurrency mechanisms
    std::mutex tree_mux;
    std::counting_semaphore<INT_MAX> semaphore{0};

    void do_work(){
        while (true) {
            semaphore.acquire();

            std::optional<T> work;
            {
                std::lock_guard<std::mutex> lock(tree_mux);
                if (stopping.load(std::memory_order_acquire) && tree.empty())
                    break;

                if (tree.empty())
                    continue;

                work.emplace(tree.pop_right_most());
            }

            (*work)();
        }
    }

    public:
        WorkPool(int nr_workers = 5)
        : nr_workers(nr_workers) {
            workers.reserve(nr_workers);
            for (int i = 0; i < nr_workers; ++i) {
                workers.emplace_back([this]() {
                    do_work();
                });
            }
        }

        ~WorkPool() {
            {
                std::lock_guard<std::mutex> lock(tree_mux);
                stopping.store(true, std::memory_order_release);
            }

            for (int i = 0; i < nr_workers; ++i)
                semaphore.release();

            for (auto& worker : workers) {
                if (worker.joinable())
                    worker.join();
            }
        }

        template<typename Func, typename... Args>
        auto submit_work(Func&& function, int priority = 0, Args&&... arguments) {

            using ReturnType = std::invoke_result_t<Func, Args...>;

            auto bound_task = [fn = std::forward<Func>(function), 
                            ...args = std::forward<Args>(arguments)]() {
                return std::invoke(std::move(fn), std::move(args)...);
            };

            auto task = std::make_shared<std::packaged_task<ReturnType()>>(std::move(bound_task));
            std::future<ReturnType> return_future = task->get_future();

            {
                std::lock_guard<std::mutex> lock(tree_mux);
                if (stopping.load(std::memory_order_acquire))
                    throw std::runtime_error("WorkPool has been stopped");

                auto queued_task = T{priority, [task]() {
                    (*task)();
                }};

                tree.put(queued_task);
            }

            semaphore.release();

            return return_future;
        }

};