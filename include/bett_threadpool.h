#ifndef BettThreadPool
#define BettThreadPool

#include <iostream>
#include <sstream>
#include <string>
#include <utility>
#include <thread>

// Resources to make a thread pool manager 
// https://github.com/methylDragon/coding-notes/blob/master/C++/07%20C++%20-%20Threading%20and%20Concurrency.md
// https://www.youtube.com/watch?v=u7ouCuieBhI
// 

unsigned int maximumActiveThreads = std::thread::hardware_concurrency();

class CBettThreadPool {
public:
    using LoggerAPI = void (*)(const std::string&);

private:
    LoggerAPI debugAPI   = nullptr;
    LoggerAPI warningAPI = nullptr;
    LoggerAPI errorAPI   = nullptr;

public:
    CBettThreadPool() = default;

    void AttachDebugAPI(LoggerAPI debugApiFunc) {
        debugAPI = debugApiFunc;
    }

    void AttachWarningAPI(LoggerAPI warningApiFunc) {
        warningAPI = warningApiFunc;
    }

    void AttachErrorAPI(LoggerAPI errorApiFunc) {
        errorAPI = errorApiFunc;
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
