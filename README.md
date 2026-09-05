# Bett

Basic ECS, Task Scheduler, Thread Pool, and UI System

<details>
  <summary><b>View Development & AI Logs</b></summary>
  <ul>
    <li>Used AI to generate the initial README structure.</li>
    <li>Fixed minor code issues and bugs.</li>
    <li>Moved code between files for better organisation.</li>
    <li>Duplicated existing code patterns where necessary.</li>
  </ul>
</details>


## Include

Include the single header or individual modules as needed:

```cpp
#include "bett.h"
#include "bett_uisystem.h"
```

Or include them separately:
```cpp
#include "bett_ecs.h"
#include "bett_scheduler.h"
#include "bett_threadpool.h"
#include "bett_uisystem.h"
```

---

## Part 1: ECS (`CBettECS`)

### Create ECS Instance

```cpp
CBettECS bett;
```

### Define Components

```cpp
struct Position {
    float x = 0.0f;
    float y = 0.0f;
};

struct Velocity {
    float vx = 0.0f;
    float vy = 0.0f;
};
```

### Create Game Object

```cpp
GameObject player = bett.CreateGameObject();
EntityID id = player.ID();
```

### Add Components

```cpp
// Via GameObject
player.AddComponent<Position>(10.0f, 20.0f);
player.AddComponent<Velocity>(1.0f, 2.0f);

// Via EntityID
bett.AddComponent<Position>(id, 10.0f, 20.0f);
```

### Check Components

```cpp
bool hasPos = player.Has<Position>();
bool hasPosById = bett.Has<Position>(id);
```

### Get Components

```cpp
Position& pos = player.GetComponent<Position>();
pos.x += 5.0f;

Position& posById = bett.GetComponent<Position>(id);
```

### Remove Components

```cpp
player.RemoveComponent<Velocity>();
bett.RemoveComponent<Velocity>(id);
```

### Check & Destroy Game Objects

```cpp
bool exists = bett.DoesGameObjectExist(player);

// Destroy
bett.RemoveGameObject(player);

// Destroy with conditional callback
bett.RemoveGameObject(player, [](EntityID entity) -> bool {
    return true;
});
```

### Query & Iterate

```cpp
// Get all active entities
const std::vector<EntityID>& entities = bett.AllEntities();

// Get all active GameObjects
std::vector<GameObject> objects = bett.AllGameObjects();

// Iterate through GameObjects
bett.forEachGameObjects([](GameObject go) {
    // Process game object
});
```

### Access Packed Component Stores

```cpp
auto& store = bett.Store<Position>();
std::vector<Position>& allPositions = store.All();
std::vector<EntityID>& allEntities = store.Entities();
```

---

## Part 2: Task Scheduler (`CBettScheduler`)

### Create Scheduler Instance

```cpp
CBettScheduler scheduler;
```

### Add Tasks

Register functions, lambdas, or member functions with arguments across stages:

```cpp
void InitOpenGL();
void InitWindow(int width, int height);
void UpdatePhysics(float dt);

// OpenGL Initialization (runs once and clears)
scheduler.AddTask(SchedulerCallOrder::OpenglInit, InitOpenGL);

// System Initialization (runs once and clears)
scheduler.AddTask(SchedulerCallOrder::SystemInit, InitWindow, 1920, 1080);

// System Update (runs every frame)
scheduler.AddTask(SchedulerCallOrder::SystemUpdate, UpdatePhysics, 0.016f);

// Render Stages
scheduler.AddTask(SchedulerCallOrder::Render3D, []() {
    // Render 3D scene
});

scheduler.AddTask(SchedulerCallOrder::Render2D, []() {
    // Render UI / overlays
});
```

### Run Tasks

```cpp
// Execute a specific stage:
scheduler.ExecuteTasks(SchedulerCallOrder::SystemUpdate);

// Or run all stages in sequence in your game loop:
while (running) {
    scheduler.Run();
}
```

---

## Part 3: Thread Pool (`CBettThreadPool`)

### Create Thread Pool Instance

```cpp
// Create a thread pool with 4 worker threads (default is 2)
CBettThreadPool pool(4);
```

### Enqueue Tasks

Submit functions or lambdas with any arguments to the thread pool:

```cpp
void ProcessData(int id, float multiplier);

// Enqueue a free function with arguments
pool.Enqueue(ProcessData, 1, 2.5f);

// Enqueue a lambda
pool.Enqueue([]() {
    // Perform heavy background calculation or asset loading
});
```

### Wait for All Tasks to Finish

Block the calling thread until all queued and active tasks complete:

```cpp
pool.WaitAll();
```

---

## Part 4: UI System (`CBettUISystem`)

### Create UI System Instance

```cpp
CBettECS bett;
CBettUISystem ui(&bett);
```

### Create UI Elements

`CreateUIElement()` automatically creates a `GameObject` and attaches `UIRectTransform`, `UIHierarchyComponent`, and `UIRenderComponent`:

```cpp
GameObject button = ui.CreateUIElement();
```

### Position & Layout (`UIRectTransform`)\n
```cpp
auto& transform = button.GetComponent<UIRectTransform>();
transform.position = glm::vec2(100.0f, 50.0f);
transform.size     = glm::vec2(220.0f, 64.0f);
```

### Visual Styling, Rounded Corners & Borders (`UIRenderComponent`)

Define idle, hover, and pressed colors, per-corner radii `(x: Top-Left, y: Top-Right, z: Bottom-Right, w: Bottom-Left)`, and borders `(x: Top, y: Right, z: Bottom, w: Left)`:

```cpp
auto& render = button.GetComponent<UIRenderComponent>();
render.normalColor = glm::vec4(0.2f, 0.5f, 0.9f, 1.0f);
render.hoverColor  = glm::vec4(0.3f, 0.65f, 1.0f, 1.0f);
render.activeColor = glm::vec4(0.1f, 0.35f, 0.7f, 1.0f);
render.radius      = glm::vec4(32.0f, 32.0f, 32.0f, 32.0f); // 32px rounded corners

// Set uniform or per-edge border thickness
render.SetBorder(2.0f); // Uniform 2px border
// or: render.SetBorder(2.0f, 4.0f, 2.0f, 4.0f); // Top, Right, Bottom, Left

// Set border colors across states
render.borderColor       = glm::vec4(0.4f, 0.7f, 1.0f, 1.0f);
render.borderHoverColor  = glm::vec4(0.8f, 0.9f, 1.0f, 1.0f);
render.borderActiveColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
render.zOrder      = 1;
```

### Images & Textures (`UIImageComponent`)

Render images/textures with optional UV clipping, color tints, rounded corners, and borders:

```cpp
auto& img = button.AddComponent<UIImageComponent>();
img.textureID = myTextureID; // OpenGL Texture ID
img.uvMin     = glm::vec2(0.0f, 0.0f);
img.uvMax     = glm::vec2(1.0f, 1.0f);
img.tint      = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
```

### User Interaction & Callbacks (`UIInteractableComponent`)

Attach interaction callbacks for click and hover events:

```cpp
auto& interact = button.AddComponent<UIInteractableComponent>();

interact.onClick = [](EntityID id) {
    std::cout << "Button clicked! ID: " << id << "\n";
};

interact.onHoverEnter = [&bett](EntityID id) {
    // Morph corner radius, border thickness, or colors dynamically on hover
    if (bett.Has<UIRenderComponent>(id)) {
        auto& render = bett.GetComponent<UIRenderComponent>(id);
        render.radius = glm::vec4(32.0f, 32.0f, 32.0f, 32.0f);
        render.SetBorder(4.0f);
    }
};

interact.onHoverExit = [&bett](EntityID id) {
    if (bett.Has<UIRenderComponent>(id)) {
        auto& render = bett.GetComponent<UIRenderComponent>(id);
        render.radius = glm::vec4(4.0f, 4.0f, 4.0f, 4.0f);
        render.SetBorder(2.0f);
    }
};
```

### UI Hierarchy (`UIHierarchyComponent`)

```cpp
GameObject panel = ui.CreateUIElement();
panel.GetComponent<UIHierarchyComponent>().children.push_back(button.ID());
button.GetComponent<UIHierarchyComponent>().parent = panel.ID();
```

### Custom UI Element Callbacks

Register custom UI update callbacks (e.g. for sliders, color pickers, input fields, scroll views):

```cpp
ui.AddCustomUIFunction([&]() {
    // Update custom UI widget logic
});
```

### Update UI in Game Loop

Call `Update()` every frame with cursor coordinates, mouse button state, and viewport dimensions. Hit-testing automatically respects corner rounding:

```cpp
while (running) {
    glm::vec2 mousePos(cursorX, cursorY);
    bool isMouseDown = CheckMouseButtonLeft();

    ui.Update(mousePos, isMouseDown, screenWidth, screenHeight);
}
```

---

## Additional: Logger API

All core Bett modules provide custom logging hooks (`LoggerAPI`), allowing you to redirect internal debug, warning, and error messages to your own logging system, console, or file sink.

### Logger Function Signature

```cpp
using LoggerAPI = void (*)(const std::string&);
```

### Attaching Custom Loggers

You can attach your custom log functions to any instance using `AttachDebugAPI`, `AttachWarningAPI`, and `AttachErrorAPI`:

```cpp
// Example: Attaching custom loggers to an instance
instance.AttachDebugAPI([](const std::string& msg) {
    std::cout << "[DEBUG] " << msg << std::endl;
});

instance.AttachWarningAPI([](const std::string& msg) {
    std::cout << "[WARN] " << msg << std::endl;
});

instance.AttachErrorAPI([](const std::string& msg) {
    std::cerr << "[ERROR] " << msg << std::endl;
});
```

If no custom logger is attached, internal messages fallback to standard output (`std::cout` for debug, `std::cerr` for warnings and errors).
