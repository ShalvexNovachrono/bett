#ifndef BettScheduler
#define BettScheduler

#include <functional>
#include <iostream>
#include <sstream>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>

#include "bett_threadpool.h"

enum SchedulerCallOrder {
    OpenglInit,
    SystemInit,
    SystemUpdate,
    GameObjectInit,
    GameObjectUpdate,
    Render3D,
    Render2D,
    Count
};

class CBettScheduler {
public:
    using LoggerAPI = void (*)(const std::string&);
    
    struct ScheduledTask {
        std::function<void()> func;
        bool requireMainThread = false;
    };

private:
    /*
    // Parent task interface
    class BTask {
    public:
        virtual ~BTask() = default;
        virtual void Execute() = 0;
    };

    // Child task holding function and packed arguments
    template <typename Func, typename... Args>
    class TaskUA : public BTask {
    private:
        Func func;
        std::tuple<Args...> args;
    public:
        TaskUA(Func callback, Args... parameters)
            : func(callback), args(std::move(parameters)...) {}

        void Execute() override {
            std::apply(func, args);
        }
    };
    */

    std::unordered_map<SchedulerCallOrder, std::vector<ScheduledTask>> tasks;
    CBettThreadPool* pool = nullptr;

    LoggerAPI debugAPI   = nullptr;
    LoggerAPI warningAPI = nullptr;
    LoggerAPI errorAPI   = nullptr;

public:
    explicit CBettScheduler(CBettThreadPool* threadPool = nullptr) : pool(threadPool) {}

    void SetThreadPool(CBettThreadPool* threadPool) {
        pool = threadPool;
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

    // Direct task overload
    void AddTask(SchedulerCallOrder stage, std::function<void()> task, bool mainThread = false) {
        tasks[stage].push_back(ScheduledTask{ std::move(task), mainThread });
    }

    // Task with parameter forwarding
    template <typename Func, typename... Args>
    void AddTask(SchedulerCallOrder stage, Func&& func, Args&&... args) {
        tasks[stage].push_back(
            ScheduledTask {
                [
                    f = std::forward<Func>(func), 
                    params = std::make_tuple(std::forward<Args>(args)...)
                ]() mutable {
                    std::apply(f, params);
                },
                false
            }
        );
    }

    void ExecuteTasks(SchedulerCallOrder stage) {
        auto it = tasks.find(stage);
        if (it == tasks.end()) return;

        bool hasAsyncTasks = false;

        for (auto& task : it->second) {
            if (!task.requireMainThread && pool != nullptr) {
                // run on the other threads
                pool->Enqueue(task.func);
                hasAsyncTasks = true;
            } else {
                // run on main thread
                task.func();
            }
        }

        // wait for all async tasks of this stage to complete before next stage begins
        if (hasAsyncTasks && pool != nullptr) {
            pool->WaitAll();
        }

        // only clear one-time init stages
        if (stage == SchedulerCallOrder::OpenglInit || 
            stage == SchedulerCallOrder::SystemInit || 
            stage == SchedulerCallOrder::GameObjectInit) {
            it->second.clear();
        }
    }

    void Run() {
        ExecuteTasks(SchedulerCallOrder::OpenglInit);
        ExecuteTasks(SchedulerCallOrder::SystemInit);
        ExecuteTasks(SchedulerCallOrder::SystemUpdate);
        ExecuteTasks(SchedulerCallOrder::GameObjectInit);
        ExecuteTasks(SchedulerCallOrder::GameObjectUpdate);
        ExecuteTasks(SchedulerCallOrder::Render3D);
        ExecuteTasks(SchedulerCallOrder::Render2D);
    }

    void WaitAll() {
        if (pool != nullptr) {
            pool->WaitAll();
        }
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
