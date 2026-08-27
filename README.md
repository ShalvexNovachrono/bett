# Bett

A basic ECS library for GameObjects and components.

## Create ECS Instance

Initializes the ECS manager.

```cpp
CBett bett;
```

## Attach Custom Warning Logger

Redirect internal Bett warnings to a custom logger or callback (defaults to `std::cerr` if omitted).

```cpp
bett.AttachWarningAPI([](const std::string& message) {
    // Forward to custom logging system
    std::cout << message << std::endl;
});
```

## Define Components

Simple data structs used by the ECS.

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

## Create Game Object

Spawns a new GameObject with a unique ID.

```cpp
GameObject player = bett.CreateGameObject();
```

## Get Entity ID

Returns the unique integer ID of the GameObject.

```cpp
EntityID id = player.ID();
```

## Add Component (GameObject)

Attaches a component to the GameObject.

```cpp
player.AddComponent<Position>(10.0f, 20.0f);
player.AddComponent<Velocity>(1.0f, 2.0f);
```

## Add Component (CBett)

Attaches a component directly via EntityID.

```cpp
bett.AddComponent<Position>(id, 10.0f, 20.0f);
```

## Check Component (GameObject)

Checks if the GameObject has a component.

```cpp
bool hasPos = player.Has<Position>();
```

## Check Component (CBett)

Checks if an EntityID has a component.

```cpp
bool hasPos = bett.Has<Position>(id);
```

## Get Component (GameObject)

Retrieves a reference to a component on the GameObject.

```cpp
Position& pos = player.GetComponent<Position>();
pos.x += 5.0f;
```

## Get Component (CBett)

Retrieves a reference to a component using EntityID.

```cpp
Position& pos = bett.GetComponent<Position>(id);
```

## Remove Component (GameObject)

Removes a component from the GameObject.

```cpp
player.RemoveComponent<Velocity>();
```

## Remove Component (CBett)

Removes a component using EntityID.

```cpp
bett.RemoveComponent<Velocity>(id);
```

## Check If Game Object Exists

Verifies whether a GameObject or EntityID is active.

```cpp
bool exists = bett.DoesGameObjectExist(player);
bool existsById = bett.DoesGameObjectExist(id);
```

## Remove Game Object

Destroys a GameObject and cleans up its components.

```cpp
bett.RemoveGameObject(player);
bett.RemoveGameObject(id);
```

## Remove Game Object With Callback

Conditionally removes a GameObject using a validation callback.

```cpp
bett.RemoveGameObject(player, [](EntityID entity) -> bool {
    return true;
});
```

## Get All Entities

Returns a const reference to the list of all active EntityIDs.

```cpp
const std::vector<EntityID>& entities = bett.AllEntities();
```

## Get All Game Objects

Returns a list of all active GameObject instances.

```cpp
std::vector<GameObject> objects = bett.AllGameObjects();
```

## Iterate Game Objects

Executes a function for each active GameObject.

```cpp
bett.forEachGameObjects([](GameObject go) {
    // Process game object
});
```

## Access Component Store

Retrieves the underlying contiguous packed container for batch updates.

```cpp
auto& store = bett.Store<Position>();
std::vector<Position>& allPositions = store.All();
std::vector<EntityID>& allEntities = store.Entities();
```
