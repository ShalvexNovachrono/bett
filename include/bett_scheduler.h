#ifndef BettScheduler
#define BettScheduler

#include "bett_ecs.h"
#include <cstdint>
#include <memory>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>

enum SchedulerCallOrder {
    SystemInit,
    SystemUpdate,
    GameObjectInit,
    GameObjectUpdate,
    Render3D,
    Render2D,
    Count
};

class CBettScheduler {
private:
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

    std::unordered_map<SchedulerCallOrder, std::vector<std::unique_ptr<BTask>>> tasks;

public:
    CBettScheduler() = default;

    template <typename Func, typename... Args>
    void AddTask(SchedulerCallOrder stage, Func func, Args&&... args) {
        tasks[stage].push_back(
            std::make_unique<TaskUA<Func, std::decay_t<Args>...>>(func, std::forward<Args>(args)...)
        );
    }

    void ExecuteTasks(SchedulerCallOrder stage) {
        auto it = tasks.find(stage);
        if (it == tasks.end()) return;

        for (auto& task : it->second) {
            task->Execute();
        }

        // Only clear one-time init stages
        if (stage == SchedulerCallOrder::SystemInit || stage == SchedulerCallOrder::GameObjectInit) {
            it->second.clear();
        }
    }

    void Run() {
        ExecuteTasks(SchedulerCallOrder::SystemInit);
        ExecuteTasks(SchedulerCallOrder::SystemUpdate);
        ExecuteTasks(SchedulerCallOrder::GameObjectInit);
        ExecuteTasks(SchedulerCallOrder::GameObjectUpdate);
        ExecuteTasks(SchedulerCallOrder::Render3D);
        ExecuteTasks(SchedulerCallOrder::Render2D);
    }
};

#endif
