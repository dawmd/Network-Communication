#ifndef __THREAD_POOL_H__
#define __THREAD_POOL_H__

#include <atomic>
#include <cstdlib>  // std::size_t
#include <thread>   // std::thread, std::thread::hardware_concurrency
#include <functional>
#include <future>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <queue>
#include <vector>
#include <type_traits>

#include "move_only_function.h"


namespace Network {


class ThreadPool {
private:
    using Task = MoveOnlyFunction<void()>;

    std::queue<Task> tasks;
    std::vector<std::thread> threads;
    std::size_t task_count;
    std::mutex mutex;
    std::atomic<bool> perform_tasks;

    constexpr static std::size_t DEFAULT_THREAD_COUNT = 6;

public:
    ThreadPool(std::size_t thread_count = std::thread::hardware_concurrency())
    : tasks{}
    , threads{}
    , task_count{0}
    , mutex{}
    , perform_tasks{true}
    {
        // std::thread::hardware_concurrency() CAN return 0
        if (!thread_count && std::thread::hardware_concurrency())
            throw std::invalid_argument{"A thread pool must have at least one thread."};

        const std::size_t count = thread_count ? thread_count : DEFAULT_THREAD_COUNT;
        for (std::size_t i = 0; i < count; ++i)
            threads.push_back(std::thread{&ThreadPool::work, this});
    }

    ThreadPool(const ThreadPool &other) = delete;

    ~ThreadPool() {
        perform_tasks = false;
        for (auto &thread : threads)
            thread.join();
    }

    template<typename F, typename... Args>
        requires std::invocable<F, Args...>
    auto add_task(F &&f, Args &&...args) {
        using R = std::invoke_result_t<std::remove_cvref_t<F>&&, std::remove_cvref_t<Args>&&...>;
        
        std::promise<R> promise;
        std::future<R> future = promise.get_future();
        
        MoveOnlyFunction<void()> task{
            [promise = std::move(promise), f = std::forward<F>(f), ...args = std::forward<Args>(args)]() mutable {
                try {
                    promise.set_value(std::invoke(std::move(f), std::forward<Args>(args)...));
                } catch (...) {
                    try {
                        promise.set_exception(std::current_exception());
                    } catch (...) {
                        // ignore
                    }
                }
            }
        };

        /* lock */ {
            const std::lock_guard<std::mutex> lock{mutex};
            tasks.push(std::move(task));
            ++task_count;
        }
        
        return future;
    }

private:
    [[nodiscard]] std::optional<Task> get_task() {
        const std::lock_guard<std::mutex> lock{mutex};
        if (task_count) {
            --task_count;
            auto result = std::make_optional(std::move(tasks.front()));
            tasks.pop();
            return result;
        } else {
            return {};
        }
    }

    void work() {
        while (perform_tasks.load()) {
            if (auto task = get_task())
                std::invoke(task.value());
            else
                std::this_thread::yield();
        }
    }
};


} // namespace Network

#endif // __THREAD_POOL_H__
