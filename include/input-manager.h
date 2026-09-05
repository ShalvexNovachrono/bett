#ifndef BettInputManager
#define BettInputManager

#include <vector>
#include <unordered_map>
#include <string>
#include <algorithm> 
#include <cmath> 
#include <glm/glm.hpp>

/// <summary>
/// Represents the state of a button or key, tracking its current and previous frame states.
/// </summary>
struct ButtonState {
    bool current = false;
    bool previous = false;

    /// <summary>
    /// Returns true if the button was pressed in the current frame.
    /// </summary>
    /// <returns>True if pressed this frame.</returns>
    bool Pressed() const { return current && !previous; }

    /// <summary>
    /// Returns true if the button was released in the current frame.
    /// </summary>
    /// <returns>True if released this frame.</returns>
    bool Released() const { return !current && previous; }

    /// <summary>
    /// Returns true if the button is currently being held down.
    /// </summary>
    /// <returns>True if held down.</returns>
    bool Held() const { return current && previous; }
};

/// <summary>
/// Defines the different types of mouse and keyboard interaction events.
/// </summary>
enum class MouseMovementState {
    MouseClick,
    MouseMotion,
    MousePassiveMotion,
    MouseScrollWheel,
    KeyboardPress
};

/// <summary>
/// Enum for special modifier keys (Control, Shift, Alt, Super, CapsLock, NumLock).
/// </summary>
enum class SpecialKeyType {
    Control = 0,
    Shift = 1,
    Alt = 2,
    Super = 3,
    CapsLock = 4,
    NumLock = 5,
    Count = 6 
};

/// <summary>
/// Represents a pair of keys used for bipolar input (e.g., positive and negative axes).
/// </summary>
struct KeyPairs {
    int keyA, keyB;
};

class CInputManager;

/// <summary>
/// Base class for input actions that calculate a delta value based on input state.
/// </summary>
class CBaseInputActionType {
public:
    virtual ~CBaseInputActionType() = default;
    CBaseInputActionType() = default;
    
    /// <summary>
    /// Calculates the delta value for the action.
    /// </summary>
    /// <param name="inputManager">Pointer to the input manager.</param>
    /// <param name="deltaTime">Reference to the frame delta time.</param>
    /// <returns>The calculated delta value.</returns>
    virtual float GetDelta(CInputManager* inputManager, float& deltaTime) = 0;
};

/// <summary>
/// An input action that calculates a smooth delta value between -1 and 1 based on key pairs.
/// </summary>
class CDeltaInputAction : public CBaseInputActionType {
    std::vector<KeyPairs> keyPairs;
    float keyPairDelta = 0.0f;
    float sensitivity = 15.0f; 
    bool disabled = false;
public:    
    CDeltaInputAction() = default;   
    ~CDeltaInputAction() override;
    
    /// <summary>
    /// Sets whether the input action is disabled.
    /// </summary>
    /// <param name="value">True to disable, false to enable.</param>
    void SetDisabled(bool value);

    /// <summary>
    /// Sets the sensitivity of the input action.
    /// </summary>
    /// <param name="value">The sensitivity value.</param>
    void SetSensitivity(float value) { sensitivity = value; }
    
    /// <summary>
    /// Checks if the input action is disabled.
    /// </summary>
    /// <returns>True if disabled.</returns>
    bool isDisabled() const;
    
    /// <summary>
    /// Adds a pair of keys to influence this delta action.
    /// </summary>
    /// <param name="pairs">The key pair (positive and negative keys).</param>
    void AddKeyPairs(KeyPairs pairs);
    
    /// <summary>
    /// Updates and returns the current delta value based on key states and sensitivity.
    /// </summary>
    /// <param name="inputManager">Pointer to the input manager.</param>
    /// <param name="deltaTime">Reference to the frame delta time.</param>
    /// <returns>The updated delta value.</returns>
    float GetDelta(CInputManager* inputManager, float& deltaTime) override;

    /// <summary>
    /// Returns the current delta value without updating it.
    /// </summary>
    /// <returns>The current delta value.</returns>
    float GetCurrentDelta() const { return keyPairDelta; }
};


/// <summary>
/// Manages input states for keyboard and mouse, providing access to button states, mouse movement, and virtual axes.
/// </summary>
class CInputManager {
private:
    float& deltaTime;
    
    static constexpr int maxMouseKeys = 7;
    static constexpr int maxKeyboardKeys = 512;

    std::unordered_map<std::string, CDeltaInputAction> deltaInputActions;
    
    std::vector<ButtonState> keyboardKeys;
    std::vector<ButtonState> mouseKeys;
    std::vector<ButtonState> specialKeys;  // Array for special modifier keys
    
    int mouseWheelDeltaX = 0;
    int mouseWheelDeltaY = 0;
    glm::vec2 mousePosition, mouseLastPosition, mouseDelta;
    MouseMovementState currentMouseMovementState = MouseMovementState::MousePassiveMotion;

    static int ClampKey(int key) {
        return (key < 0) ? -1 : (key >= maxKeyboardKeys ? -1 : key);
    }

    static int ClampMouseKey(int key) {
        return (key < 0) ? -1 : (key >= maxMouseKeys ? -1 : key);
    }

public:
    /// <summary>
    /// Initializes a new instance of the CInputManager class.
    /// </summary>
    /// <param name="DeltaTime">Reference to the global delta time.</param>
    CInputManager(float& DeltaTime);

    /// <summary>
    /// Updates internal states at the beginning of a frame. Should be called once per frame.
    /// </summary>
    void BeginFrame();

    /// <summary>
    /// Updates internal states at the end of a frame. Should be called once per frame.
    /// </summary>
    void EndFrame();

    /// <summary>
    /// Updates the state of a keyboard key.
    /// </summary>
    /// <param name="key">The key code.</param>
    /// <param name="isDown">True if the key is pressed.</param>
    /// <param name="cursorPosition">The mouse position at the time of the event.</param>
    /// <param name="state">The movement state associated with the event.</param>
    void SetKeyState(int key, bool isDown, glm::vec2 cursorPosition, MouseMovementState state);

    /// <summary>
    /// Checks if a key is currently held down.
    /// </summary>
    /// <param name="key">The key code.</param>
    /// <returns>True if the key is down.</returns>
    bool IsDown(int key) const;

    /// <summary>
    /// Checks if a key was just pressed this frame.
    /// </summary>
    /// <param name="key">The key code.</param>
    /// <returns>True if the key was just pressed.</returns>
    bool WasPressed(int key) const;

    /// <summary>
    /// Checks if a key was just released this frame.
    /// </summary>
    /// <param name="key">The key code.</param>
    /// <returns>True if the key was just released.</returns>
    bool WasReleased(int key) const;

    /// <summary>
    /// Updates the mouse position.
    /// </summary>
    /// <param name="position">The new mouse position.</param>
    /// <param name="state">The movement state associated with the change.</param>
    void SetMousePosition(glm::vec2 position, MouseMovementState state);

    /// <summary>
    /// Updates the state of a mouse button.
    /// </summary>
    /// <param name="key">The mouse button index (0 = left, 1 = middle, 2 = right).</param>
    /// <param name="isDown">True if the button is pressed.</param>
    /// <param name="cursorPosition">The mouse position at the time of the event.</param>
    /// <param name="state">The movement state associated with the event.</param>
    void SetMouseButtonState(int key, bool isDown, glm::vec2 cursorPosition, MouseMovementState state);

    /// <summary>
    /// Accumulates mouse wheel movement.
    /// </summary>
    /// <param name="deltaX">The scroll amount.</param>
    /// <param name="deltaY">The scroll amount.</param>
    /// <param name="cursorPosition">The mouse position at the time of the scroll.</param>
    void AddMouseWheelDelta(int deltaX, int deltaY, glm::vec2 cursorPosition);

    /// <summary>
    /// Returns the current mouse position.
    /// </summary>
    /// <returns>The mouse position as a vec2.</returns>
    glm::vec2 GetMousePosition() const;

    /// <summary>
    /// Checks if a mouse button is currently held down.
    /// </summary>
    /// <param name="key">The mouse button index (0 = left, 1 = middle, 2 = right).</param>
    /// <returns>True if the mouse button is down.</returns>
    bool IsMouseButtonDown(int key) const;

    bool IsAnyMouseButtonDown() const;

    /// <summary>
    /// Checks if a mouse button was just pressed this frame.
    /// </summary>
    /// <param name="key">The mouse button index.</param>
    /// <returns>True if the button was just pressed.</returns>
    bool WasMouseButtonPressed(int key) const;

    /// <summary>
    /// Checks if a mouse button was just released this frame.
    /// </summary>
    /// <param name="key">The mouse button index.</param>
    /// <returns>True if the button was just released.</returns>
    bool WasMouseButtonReleased(int key) const;

    /// <summary>
    /// Returns the mouse movement delta since the last frame.
    /// </summary>
    /// <returns>The mouse delta as a vec2.</returns>
    glm::vec2 GetMouseDelta() const;

    /// <summary>
    /// Returns the current state of mouse interaction.
    /// </summary>
    /// <returns>The current MouseMovementState.</returns>
    MouseMovementState GetMouseMovementState() const;

    /// <summary>
    /// Returns a string representation of the current mouse movement state.
    /// </summary>
    /// <returns>A string describing the mouse state.</returns>
    std::string GetMouseMovementStateString() const;

    
    /// <summary>
    /// Returns the accumulated mouse wheel scroll delta x for the current frame.
    /// </summary>
    /// <returns>The scroll delta.</returns>
    int GetMouseWheelDeltaX() const;

    /// <summary>
    /// Returns the accumulated mouse wheel scroll delta y for the current frame.
    /// </summary>
    /// <returns>The scroll delta.</returns>
    int GetMouseWheelDeltaY() const;
    
    /// <summary>
    /// Adds or updates a delta input action (virtual axis) controlled by key pairs.
    /// </summary>
    /// <param name="actionName">The name of the action.</param>
    /// <param name="pairs">The key pair that controls the action.</param>
    void AddDeltaInputAction(std::string actionName, KeyPairs pairs);

    /// <summary>
    /// Sets the sensitivity for a specific delta input action.
    /// </summary>
    /// <param name="actionName">The name of the action.</param>
    /// <param name="sensitivity">The new sensitivity value.</param>
    void SetDeltaInputSensitivity(std::string actionName, float sensitivity);

    /// <summary>
    /// Returns the current value of a delta input action.
    /// </summary>
    /// <param name="actionName">The name of the action.</param>
    /// <returns>The delta value (typically between -1 and 1).</returns>
    float GetDeltaInputAction(std::string actionName);

    /// <summary>
    /// Sets the visibility of the mouse cursor.
    /// </summary>
    /// <param name="visible">True to make the cursor visible, false to hide it.</param>
    void SetCursorVisible(bool visible);

    /// <summary>
    /// Sets the style of the mouse cursor.
    /// </summary>
    /// <param name="cursorStyle">The GLUT cursor style constant.</param>
    void SetCursorStyle(int cursorStyle);

    /// <summary>
    /// Sets the mouse cursor to a right arrow.
    /// </summary>
    void SetCursorRightArrow();

    /// <summary>
    /// Sets the mouse cursor to a left arrow.
    /// </summary>
    void SetCursorLeftArrow();

    /// <summary>
    /// Sets the mouse cursor to an info icon.
    /// </summary>
    void SetCursorInfo();

    /// <summary>
    /// Sets the mouse cursor to a destroy icon.
    /// </summary>
    void SetCursorDestroy();

    /// <summary>
    /// Sets the mouse cursor to a help icon.
    /// </summary>
    void SetCursorHelp();

    /// <summary>
    /// Sets the mouse cursor to a cycle icon.
    /// </summary>
    void SetCursorCycle();

    /// <summary>
    /// Sets the mouse cursor to a spray icon.
    /// </summary>
    void SetCursorSpray();

    /// <summary>
    /// Sets the mouse cursor to a wait icon.
    /// </summary>
    void SetCursorWait();

    /// <summary>
    /// Sets the mouse cursor to a text (I-beam) icon.
    /// </summary>
    void SetCursorText();

    /// <summary>
    /// Sets the mouse cursor to a crosshair.
    /// </summary>
    void SetCursorCrosshair();

    /// <summary>
    /// Sets the mouse cursor to be invisible.
    /// </summary>
    void SetCursorNone();

    /// <summary>
    /// Sets the mouse cursor to inherit the default system cursor.
    /// </summary>
    void SetCursorInherit();

    /// <summary>
    /// Updates the state of a special modifier key (Control, Shift, Alt, Super, CapsLock, NumLock).
    /// </summary>
    /// <param name="specialKey">The special key type to update.</param>
    /// <param name="isDown">True if the key is pressed.</param>
    void SetSpecialKeyState(SpecialKeyType specialKey, bool isDown);

    /// <summary>
    /// Checks if a special key is currently held down.
    /// </summary>
    /// <param name="specialKey">The special key type to check.</param>
    /// <returns>True if the key is down.</returns>
    bool IsSpecialKeyDown(SpecialKeyType specialKey) const;

    /// <summary>
    /// Checks if a special key was just pressed this frame.
    /// </summary>
    /// <param name="specialKey">The special key type to check.</param>
    /// <returns>True if the key was just pressed.</returns>
    bool WasSpecialKeyPressed(SpecialKeyType specialKey) const;

    /// <summary>
    /// Checks if a special key was just released this frame.
    /// </summary>
    /// <param name="specialKey">The special key type to check.</param>
    /// <returns>True if the key was just released.</returns>
    bool WasSpecialKeyReleased(SpecialKeyType specialKey) const;

    /// <summary>
    /// Checks if one or more special keys are currently pressed.
    /// </summary>
    /// <param name="specialKeys">Array of special key types to check.</param>
    /// <param name="count">Number of keys in the array.</param>
    /// <returns>True if all specified keys are currently pressed.</returns>
    bool AreSpecialKeysPressed(const SpecialKeyType* specialKeys, int count) const;

    /// <summary>
    /// Checks if any one of the specified special keys are currently pressed.
    /// </summary>
    /// <param name="specialKeys">Array of special key types to check.</param>
    /// <param name="count">Number of keys in the array.</param>
    /// <returns>True if any of the specified keys are currently pressed.</returns>
    bool IsAnySpecialKeyPressed(const SpecialKeyType* specialKeys, int count) const;
};

inline CDeltaInputAction::~CDeltaInputAction() {
    keyPairs.clear();
}

inline void CDeltaInputAction::SetDisabled(bool value) {
    disabled = value;
}

inline bool CDeltaInputAction::isDisabled() const {
    return disabled;
}

inline void CDeltaInputAction::AddKeyPairs(KeyPairs pairs) {
    keyPairs.push_back(pairs);   
}

inline float CDeltaInputAction::GetDelta(CInputManager* inputManager, float& deltaTime) {
    if (disabled) return 0.0f;

    bool pos = false, neg = false;
    for (int i = 0; i < static_cast<int>(keyPairs.size()); i++) {
        const auto& kp = keyPairs[i];
        if (inputManager->IsDown(kp.keyA)) pos = true;
        else if (inputManager->IsDown(kp.keyB)) neg = true;
    }

    if (pos) {
        keyPairDelta += sensitivity * deltaTime;
    }
    else if (neg) {
        keyPairDelta -= sensitivity * deltaTime;
    }
    else if (keyPairDelta != 0.0f) {
        float s = (keyPairDelta > 0.0f ? 1.0f : -1.0f);
        keyPairDelta -= s * sensitivity * deltaTime;
        if (std::abs(keyPairDelta) < (sensitivity * deltaTime * 1.1f)) keyPairDelta = 0.0f;
    }

    keyPairDelta = std::clamp<float>(keyPairDelta, -1.0f, 1.0f);
    return keyPairDelta;
}

inline CInputManager::CInputManager(float& DeltaTime) : deltaTime(DeltaTime) {
    keyboardKeys.reserve(maxKeyboardKeys);
    for (int i = 0; i < maxKeyboardKeys; i++) {
        keyboardKeys.push_back(ButtonState{});
    }

    mouseKeys.reserve(maxMouseKeys);
    for (int i = 0; i < maxMouseKeys; i++) {
        mouseKeys.push_back(ButtonState{});
    }

    // Initialize special keys array (Control, Shift, Alt, Super, CapsLock, NumLock)
    int specialKeyCount = static_cast<int>(SpecialKeyType::Count);
    specialKeys.reserve(specialKeyCount);
    for (int i = 0; i < specialKeyCount; i++) {
        specialKeys.push_back(ButtonState{});
    }

    mousePosition = glm::vec2(0.0f, 0.0f);
    mouseLastPosition = glm::vec2(0.0f, 0.0f);
    mouseDelta = glm::vec2(0.0f, 0.0f);
}

inline void CInputManager::BeginFrame() {
    mouseDelta = mousePosition - mouseLastPosition;
    mouseLastPosition = mousePosition;

    for (int i = 0; i < static_cast<int>(mouseKeys.size()); i++) {
        mouseKeys[i].previous = mouseKeys[i].current;
    }

    for (auto& kv : deltaInputActions) {
        kv.second.GetDelta(this, deltaTime);
    }
}

inline void CInputManager::EndFrame() {
    for (int i = 0; i < static_cast<int>(keyboardKeys.size()); i++) {
        keyboardKeys[i].previous = keyboardKeys[i].current;
    }
    
    for (int i = 0; i < static_cast<int>(specialKeys.size()); i++) {
        specialKeys[i].previous = specialKeys[i].current;
    }
    
    mouseWheelDeltaX = 0;
    mouseWheelDeltaY = 0;
}

inline void CInputManager::SetKeyState(int key, bool isDown, glm::vec2 cursorPosition, MouseMovementState state) {
    key = ClampKey(key);
    if (key < 0) return;
    if (key >= static_cast<int>(keyboardKeys.size())) return;

    keyboardKeys[key].current = isDown;
    mousePosition = cursorPosition;
    currentMouseMovementState = state;
}

inline bool CInputManager::IsDown(int key) const {
    key = ClampKey(key);
    if (key < 0) return false;
    if (key >= static_cast<int>(keyboardKeys.size())) return false;

    return keyboardKeys[key].current;
}

inline bool CInputManager::WasPressed(int key) const {
    key = ClampKey(key);
    if (key < 0) return false;
    if (key >= static_cast<int>(keyboardKeys.size())) return false;

    return keyboardKeys[key].Pressed();
}

inline bool CInputManager::WasReleased(int key) const {
    key = ClampKey(key);
    if (key < 0) return false;
    if (key >= static_cast<int>(keyboardKeys.size())) return false;

    return keyboardKeys[key].Released();
}

inline void CInputManager::SetMousePosition(glm::vec2 position, MouseMovementState state) {
    mousePosition = position;
    currentMouseMovementState = state;
}

inline void CInputManager::SetMouseButtonState(int key, bool isDown, glm::vec2 cursorPosition, MouseMovementState state) {
    key = ClampMouseKey(key);
    if (key < 0) return;
    if (key >= static_cast<int>(mouseKeys.size())) return;

    mouseKeys[key].current = isDown;
    mousePosition = cursorPosition;
    currentMouseMovementState = state;
}

inline void CInputManager::AddMouseWheelDelta(int deltaX, int deltaY, glm::vec2 cursorPosition) {
    mouseWheelDeltaX += deltaX;
    mouseWheelDeltaY += deltaY;
    mousePosition = cursorPosition;
    currentMouseMovementState = MouseMovementState::MouseScrollWheel;
}

inline glm::vec2 CInputManager::GetMousePosition() const {
    return mousePosition;
}

inline bool CInputManager::IsMouseButtonDown(int key) const {
    key = ClampMouseKey(key);
    if (key < 0) return false;
    if (key >= static_cast<int>(mouseKeys.size())) return false;

    return mouseKeys[key].current;
}

inline bool CInputManager::IsAnyMouseButtonDown() const {
    for (const auto& key : mouseKeys) {
        if (key.current) {
            return true;
        }
    }
    return false;
}

inline bool CInputManager::WasMouseButtonPressed(int key) const {
    key = ClampMouseKey(key);
    if (key < 0) return false;
    if (key >= static_cast<int>(mouseKeys.size())) return false;

    return mouseKeys[key].Pressed();
}

inline bool CInputManager::WasMouseButtonReleased(int key) const {
    key = ClampMouseKey(key);
    if (key < 0) return false;
    if (key >= static_cast<int>(mouseKeys.size())) return false;

    return mouseKeys[key].Released();
}

inline glm::vec2 CInputManager::GetMouseDelta() const {    
    return mouseDelta;
}

inline MouseMovementState CInputManager::GetMouseMovementState() const {
    return currentMouseMovementState;
}

inline std::string CInputManager::GetMouseMovementStateString() const {
    switch (currentMouseMovementState) {
        case MouseMovementState::MouseClick: return "MouseClick";
        case MouseMovementState::MouseMotion: return "MouseMotion";
        case MouseMovementState::MousePassiveMotion: return "MousePassiveMotion";
        case MouseMovementState::MouseScrollWheel: return "MouseScrollWheel";
        case MouseMovementState::KeyboardPress: return "KeyboardPress";
        default: return "Unknown";
    }
}

inline int CInputManager::GetMouseWheelDeltaX() const {
    return mouseWheelDeltaX;
}

inline int CInputManager::GetMouseWheelDeltaY() const {
    return mouseWheelDeltaY;
}

inline void CInputManager::AddDeltaInputAction(std::string actionName, KeyPairs pairs) {
    auto it = deltaInputActions.find(actionName);
    if (it != deltaInputActions.end()) {
        it->second.AddKeyPairs(pairs);
    } else {
        CDeltaInputAction newAction;
        newAction.AddKeyPairs(pairs);
        deltaInputActions[actionName] = newAction;
    }
}

inline void CInputManager::SetDeltaInputSensitivity(std::string actionName, float sensitivity) {
    auto it = deltaInputActions.find(actionName);
    if (it != deltaInputActions.end()) {
        it->second.SetSensitivity(sensitivity);
    }
}

inline float CInputManager::GetDeltaInputAction(std::string actionName) {
    auto it = deltaInputActions.find(actionName);
    if (it != deltaInputActions.end()) {
        return it->second.GetCurrentDelta();
    }
    return 0.0f;
}

inline void CInputManager::SetSpecialKeyState(SpecialKeyType specialKey, bool isDown) {
    int index = static_cast<int>(specialKey);
    if (index < 0 || index >= static_cast<int>(specialKeys.size())) return;
    specialKeys[index].current = isDown;
}

inline bool CInputManager::IsSpecialKeyDown(SpecialKeyType specialKey) const {
    int index = static_cast<int>(specialKey);
    if (index < 0 || index >= static_cast<int>(specialKeys.size())) return false;
    return specialKeys[index].current;
}

inline bool CInputManager::WasSpecialKeyPressed(SpecialKeyType specialKey) const {
    int index = static_cast<int>(specialKey);
    if (index < 0 || index >= static_cast<int>(specialKeys.size())) return false;
    return specialKeys[index].Pressed();
}

inline bool CInputManager::WasSpecialKeyReleased(SpecialKeyType specialKey) const {
    int index = static_cast<int>(specialKey);
    if (index < 0 || index >= static_cast<int>(specialKeys.size())) return false;
    return specialKeys[index].Released();
}

inline bool CInputManager::AreSpecialKeysPressed(const SpecialKeyType* specialKeys, int count) const {
    if (!specialKeys || count <= 0) return false;
    
    for (int i = 0; i < count; i++) {
        if (!IsSpecialKeyDown(specialKeys[i])) {
            return false;
        }
    }
    return true;
}

inline bool CInputManager::IsAnySpecialKeyPressed(const SpecialKeyType* specialKeys, int count) const {
    if (!specialKeys || count <= 0) return false;
    
    for (int i = 0; i < count; i++) {
        if (IsSpecialKeyDown(specialKeys[i])) {
            return true;
        }
    }
    return false;
}

#endif 