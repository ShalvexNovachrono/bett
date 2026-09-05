#ifndef BettWindowManager
#define BettWindowManager

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include <bett_threadpool.h>
#include <bett_scheduler.h>
#include <input-manager.h>


class CWindowManager;

namespace GLCallbacks {
    namespace {
        CInputManager* inputManagerInstance = nullptr;
        CWindowManager* windowManagerInstance = nullptr;
    }
    void KeyBoardCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    void MouseMotionCallback(GLFWwindow* window, double xpos, double ypos);
    void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    void MouseScrollWheelCallback(GLFWwindow* window, double xoffset, double yoffset);
    void InitWindowManagerInstance(CWindowManager* wm);
    void InitInputManagerInstance(CInputManager* input);
    void ResizeWindow(GLFWwindow *window, int width, int height);
}

class CWindowManager {
public:
    using LoggerAPI = void (*)(const std::string&);

public:
    CWindowManager(int width, int height, const std::string& title) {
        this->width = width;
        this->height = height;
        this->title = title;
        scheduler = std::make_unique<CBettScheduler>();
    }
    
    virtual ~CWindowManager() {
        if (window != nullptr) {
            glfwDestroyWindow(window);
            window = nullptr;
        }
        glfwTerminate();
    }

    void Init() {
        /* --- Initialize the library --- */
        if (!glfwInit()) {
            BettBLogError("GLFW failed to initialize.");
        }

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
        window = glfwCreateWindow(width, height, title.c_str(), NULL, NULL);
        input = std::make_unique<CInputManager>(deltaTime);
        pool = std::make_unique<CBettThreadPool>(MAX_THREAD);
        scheduler->SetThreadPool(pool.get());
    
        if (!window) {
            glfwTerminate();
            BettBLogWarning("Failed to create window.");
        }
    
        glfwMakeContextCurrent(window);
    
        glewExperimental = GL_TRUE;
        if (glewInit() != GLEW_OK) {
            BettBLogError("GLEW failed to initialize.");
        }

        // Setup OpenGL states
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
        glfwSwapInterval(0); // Unlimited FPS for real-time benchmark
    
        /* --- Initialize the callback namespace --- */
        GLCallbacks::InitWindowManagerInstance(this);
        GLCallbacks::InitInputManagerInstance(input.get());
    
        /* --- Setup the bindings --- */
        glfwSetFramebufferSizeCallback(window, GLCallbacks::ResizeWindow);
        glfwSetKeyCallback(window, GLCallbacks::KeyBoardCallback);
        glfwSetCursorPosCallback(window, GLCallbacks::MouseMotionCallback);
        glfwSetMouseButtonCallback(window, GLCallbacks::MouseButtonCallback);
        glfwSetScrollCallback(window, GLCallbacks::MouseScrollWheelCallback);
    
        Start();
    
        glfwSetTime(0.0);
    
        Tick();
    }

    void ResizeWindow(GLFWwindow* window, int width, int height) {
        this->width = width;
        this->height = height;
        aspectRatio = static_cast<float>(width) / static_cast<float>(height);
        glViewport(0, 0, width, height);
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

    void AddTask(SchedulerCallOrder stage, std::function<void()> task, bool mainThread = false) {
        if (!scheduler) {
            scheduler = std::make_unique<CBettScheduler>(pool ? pool.get() : nullptr);
        }
        scheduler->AddTask(stage, std::move(task), mainThread);
    }

    CInputManager* GetInputManager() const { return input.get(); }
    CBettScheduler* GetScheduler() const { return scheduler.get(); }
    CBettThreadPool* GetThreadPool() const { return pool.get(); }
    GLFWwindow* GetGLFWWindow() const { return window; }
    float GetDeltaTime() const { return deltaTime; }
    int GetFPS() const { return fps; }
    int GetWidth() const { return width; }
    int GetHeight() const { return height; }

private:
    // Logger API GLCallbacks
    LoggerAPI debugAPI   = nullptr;
    LoggerAPI warningAPI = nullptr;
    LoggerAPI errorAPI   = nullptr;

    // Window Properties
    int width = 1280;
    int height = 720;
    std::string title = "Nava's Window Manager";
    GLFWwindow* window = nullptr;

    // Time & Framerate
    float deltaTime = 0.0f;
    float systemDeltaTime = 0.0f;
    float lastFrame = 0.0f;
    float currentFrame = 0.0f;
    float timeScale = 1.0f;
    int fps = 0;
    float frameRate = 60.0f;
    int frameTime = static_cast<int>(1000.0f / frameRate);

    // Camera & Projection Settings
    float fov = 45.0f;
    const float minimumFov = 1.0f;
    const float maximumFov = 110.0f;
    float aspectRatio = 16.0f / 9.0f;
    float renderClippingNear = 0.1f;
    float renderClippingFar = 1000.0f;
    float mouseSensitivity = 0.1f;

    // Render Settings
    glm::mat4 model = glm::mat4(1.0f);
    glm::vec4 clearColor = glm::vec4(0.06f, 0.07f, 0.10f, 1.0f);

    // System Data
    size_t MAX_THREAD = 4;
    
    // Systems Managers
    std::unique_ptr<CInputManager> input;
    std::unique_ptr<CBettThreadPool> pool;
    std::unique_ptr<CBettScheduler> scheduler;

protected:
    virtual void Start() {}

private:
    // Internal Lifecycle & Calculations
    void CalculateDeltaTime() {
        currentFrame = static_cast<float>(glfwGetTime());
        systemDeltaTime = currentFrame - lastFrame;
        if (systemDeltaTime <= 0.0f) systemDeltaTime = 1e-6f;
        deltaTime = timeScale * systemDeltaTime;
        lastFrame = currentFrame;
        fps = static_cast<int>(1.0f / systemDeltaTime);
    }
    
    void Tick() {
        while (!glfwWindowShouldClose(window)) {
            CalculateDeltaTime();
    
            glfwPollEvents();

            scheduler->ExecuteTasks(SchedulerCallOrder::SystemUpdate);
            scheduler->ExecuteTasks(SchedulerCallOrder::GameObjectUpdate);

            // Update should be called here can run on the all the threads unless it usees on opengl funcs
    
            glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


            // Draw call should be here; should only run on the main thread
            glfwSwapBuffers(window);
        }
    }

private:
    // Internal Logger

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

// GLCallbacks

inline void GLCallbacks::KeyBoardCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (inputManagerInstance != nullptr) {

        // Convert GLFW action to boolean (GLFW_PRESS = 1, GLFW_RELEASE = 0, GLFW_REPEAT = 2)
        bool isKeyDown = (action != GLFW_RELEASE);
        
        // Get current cursor position (safe if window is null in headless/tests)
        glm::vec2 cursorPosition = inputManagerInstance->GetMousePosition();
        if (window != nullptr) {
            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);
            cursorPosition = glm::vec2(static_cast<float>(xpos), static_cast<float>(ypos));
        }

        // Set key state
        inputManagerInstance->SetKeyState(key, isKeyDown, cursorPosition, MouseMovementState::KeyboardPress);

        // Update special key states
        inputManagerInstance->SetSpecialKeyState(SpecialKeyType::Control, (mods & GLFW_MOD_CONTROL) != 0);
        inputManagerInstance->SetSpecialKeyState(SpecialKeyType::Shift, (mods & GLFW_MOD_SHIFT) != 0);
        inputManagerInstance->SetSpecialKeyState(SpecialKeyType::Alt, (mods & GLFW_MOD_ALT) != 0);
        inputManagerInstance->SetSpecialKeyState(SpecialKeyType::Super, (mods & GLFW_MOD_SUPER) != 0);
        inputManagerInstance->SetSpecialKeyState(SpecialKeyType::CapsLock, (mods & GLFW_MOD_CAPS_LOCK) != 0);
        inputManagerInstance->SetSpecialKeyState(SpecialKeyType::NumLock, (mods & GLFW_MOD_NUM_LOCK) != 0);
    }
}

inline void GLCallbacks::MouseMotionCallback(GLFWwindow* window, double xpos, double ypos) {
    if (inputManagerInstance != nullptr) {
        glm::vec2 cursorPosition = glm::vec2(static_cast<float>(xpos), static_cast<float>(ypos));
        MouseMovementState state = inputManagerInstance->IsAnyMouseButtonDown()
            ? MouseMovementState::MouseMotion
            : MouseMovementState::MousePassiveMotion;
        inputManagerInstance->SetMousePosition(cursorPosition, state);
    }
}

inline void GLCallbacks::MouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (inputManagerInstance != nullptr) {
        bool isDown = (action != GLFW_RELEASE);
        glm::vec2 cursorPosition = inputManagerInstance->GetMousePosition();
        if (window != nullptr) {
            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);
            cursorPosition = glm::vec2(static_cast<float>(xpos), static_cast<float>(ypos));
        }

        inputManagerInstance->SetMouseButtonState(button, isDown, cursorPosition, MouseMovementState::MouseClick);

        inputManagerInstance->SetSpecialKeyState(SpecialKeyType::Control, (mods & GLFW_MOD_CONTROL) != 0);
        inputManagerInstance->SetSpecialKeyState(SpecialKeyType::Shift, (mods & GLFW_MOD_SHIFT) != 0);
        inputManagerInstance->SetSpecialKeyState(SpecialKeyType::Alt, (mods & GLFW_MOD_ALT) != 0);
        inputManagerInstance->SetSpecialKeyState(SpecialKeyType::Super, (mods & GLFW_MOD_SUPER) != 0);
        inputManagerInstance->SetSpecialKeyState(SpecialKeyType::CapsLock, (mods & GLFW_MOD_CAPS_LOCK) != 0);
        inputManagerInstance->SetSpecialKeyState(SpecialKeyType::NumLock, (mods & GLFW_MOD_NUM_LOCK) != 0);
    }
}

inline void GLCallbacks::MouseScrollWheelCallback(GLFWwindow* window, double xoffset, double yoffset) {
    if (inputManagerInstance != nullptr) {
        glm::vec2 cursorPosition = inputManagerInstance->GetMousePosition();
        if (window != nullptr) {
            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);
            cursorPosition = glm::vec2(static_cast<float>(xpos), static_cast<float>(ypos));
        }
        inputManagerInstance->AddMouseWheelDelta(static_cast<int>(xoffset), static_cast<int>(yoffset), cursorPosition);
    }
}

inline void GLCallbacks::InitWindowManagerInstance(CWindowManager* wm) {
    windowManagerInstance = wm;
}

inline void GLCallbacks::InitInputManagerInstance(CInputManager* input) {
    inputManagerInstance = input;
}

inline void GLCallbacks::ResizeWindow(GLFWwindow* window, int width, int height) {
    if (windowManagerInstance != nullptr) {
        windowManagerInstance->ResizeWindow(window, width, height);
    }
}

#endif
