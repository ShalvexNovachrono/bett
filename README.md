# Bett

Basic ECS and Task Scheduler

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

Include the single header to access both ECS and Scheduler:

```cpp
#include "bett.h"
```

Or include them separately:
```cpp
#include "bett_ecs.h"
#include "bett_scheduler.h"
```

---

## Part 1: ECS (`CBettECS`)

### Create ECS Instance

```cpp
CBettECS bett;
```

### Attach Custom Loggers (Optional)

Redirect internal Bett logs to your own logger:

```cpp
bett.AttachDebugAPI([](const std::string& msg) {
    std::cout << "[DEBUG] " << msg << std::endl;
});

bett.AttachWarningAPI([](const std::string& msg) {
    std::cout << "[WARN] " << msg << std::endl;
});

bett.AttachErrorAPI([](const std::string& msg) {
    std::cerr << "[ERROR] " << msg << std::endl;
});
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
void InitWindow(int width, int height);
void UpdatePhysics(float dt);

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
