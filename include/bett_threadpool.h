#ifndef BettThreadPool
#define BettThreadPool

#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <functional>
#include <iostream>
#include <mutex>
#include <queue>
#include <sstream>
#include <string>
#include <thread>
#include <tuple>
#include <utility>
#include <vector>

// Resources to make a thread pool manager 
// https://github.com/methylDragon/coding-notes/blob/master/C++/07%20C++%20-%20Threading%20and%20Concurrency.md
// https://www.youtube.com/watch?v=u7ouCuieBhI 
// https://www.youtube.com/watch?v=BedUiLRDOKo 


class CBettThreadPool {
public:
    using LoggerAPI = void (*)(const std::string&);

private:
    LoggerAPI debugAPI   = nullptr;
    LoggerAPI warningAPI = nullptr;
    LoggerAPI errorAPI   = nullptr;

    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;

    std::mutex queueTaskMutex;
    std::condition_variable cvTaskAvailable; // Wakes workers when tasks arrive
    std::condition_variable cvAllIdle;       // Wakes WaitAll() when everything is done

    // the main task runs on the main thread
    std::mutex mainQueueMutex;
    std::queue<std::function<void()>> mainTask;
    
    std::atomic<bool> stopFlag{false};
    std::atomic<size_t> activeTasks{0};

public:
    CBettThreadPool(size_t numberThreads = 2) {
        for (size_t i = 0; i < numberThreads; i++) {
            workers.emplace_back(
                [this] () {
                    // for (;;) means loop for every
                    for (;;) {
                        std::unique_lock<std::mutex> lock(queueTaskMutex);

                        cvTaskAvailable.wait(lock, [this] { return stopFlag.load() || !tasks.empty(); });

                        if (stopFlag.load() && tasks.empty()) return;

                        auto task = std::move(tasks.front()); // get task
                        tasks.pop(); // remove task from queue
                        activeTasks.fetch_add(1);
                        
                        lock.unlock(); 

                        task();

                        activeTasks.fetch_sub(1);
                        {
                            std::unique_lock<std::mutex> lk(queueTaskMutex);
                            if (tasks.empty() && activeTasks.load() == 0) {
                                cvAllIdle.notify_all();
                            }
                        }
                    }
                }
            );
        }
    }

    ~CBettThreadPool() {
        {
            std::unique_lock<std::mutex> lock(queueTaskMutex);
            stopFlag = true;
        }
        cvTaskAvailable.notify_all();

        for (std::thread& worker : workers) {
            if (worker.joinable()) {
                worker.join();
            }
        }
    }

    void AttachDebugAPI(LoggerAPI debugApiFunc) {
        debugAPI = debugApiFunc;
    }

    void AttachWarningAPI(LoggerAPI warningApiFunc) {
        warningAPI = warningApiFunc;
    }

    void AttachErrorAPI(LoggerAPI errorApiFunc) {
        errorAPI = errorApiFunc;
    }

    template <typename Func, typename... Args>
    void Enqueue(Func&& func, Args&&... args) {
        {
            std::unique_lock<std::mutex> lock(queueTaskMutex);

            tasks.emplace(
                [
                    func = std::forward<Func>(func),
                    args = std::make_tuple(std::forward<Args>(args)...)
                ] 
                () mutable
                {
                    std::apply(func, args);
                }
            );
        }
        cvTaskAvailable.notify_one();
    }

    template <typename Func, typename... Args>
    void EnqueueMainThread(Func&& func, Args&&... args) {
        {
            std::unique_lock<std::mutex> lock(mainQueueMutex);

            mainTask.emplace(
                [
                    func = std::forward<Func>(func),
                    args = std::make_tuple(std::forward<Args>(args)...)
                ] 
                () mutable
                {
                    std::apply(func, args);
                }
            );
        }
    }

    void ExecuteMainThreadTasks() {
        std::queue<std::function<void()>> toRun;
        {
            std::lock_guard<std::mutex> lock(mainQueueMutex);
            std::swap(toRun, mainTask);
        }
        while (!toRun.empty()) {
            toRun.front()(); // gets the task from the front of queue and runs it hence the () after the first ()
            toRun.pop(); // removes it
        }
    }

    void WaitAll() {
        std::unique_lock<std::mutex> lock(queueTaskMutex);
        
        cvAllIdle.wait(lock, [this] {
            return tasks.empty() && activeTasks.load() == 0;
        });
    }

private:
    // Internal Logger //

    template <typename... Args>
    void BettBLogDebug(Args&&... args) {
        std::ostringstream oss;
        oss << "[Bett Debug] ";

        (oss << ... << std::forward<Args>(args));

        if (debugAPI != nullptr)
            debugAPI(oss.str());
        else
            std::cout << oss.str() << "\n";
    }

    template <typename... Args>
    void BettBLogWarning(Args&&... args) {
        std::ostringstream oss;
        // https://stackoverflow.com/questions/12233710/how-do-i-use-the-ostringstream-properly-in-c
        oss << "[Bett Warning] ";

        (oss << ... << std::forward<Args>(args));

        if (warningAPI != nullptr)
            warningAPI(oss.str());
        else
            std::cerr << oss.str() << "\n";
    }

    template <typename... Args>
    void BettBLogError(Args&&... args) {
        std::ostringstream oss;
        oss << "[Bett Error] ";

        (oss << ... << std::forward<Args>(args));

        if (errorAPI != nullptr)
            errorAPI(oss.str());
        else
            std::cerr << oss.str() << "\n";
    }
};

#endif
