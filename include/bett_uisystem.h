#ifndef BettUISystem
#define BettUISystem

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "bett_ecs.h"

namespace Bett {
    namespace detail {
        struct Vec2 {
            float x{0.0f};
            float y{0.0f};

            Vec2() = default;
            constexpr explicit Vec2(float scalar) : x(scalar), y(scalar) {}
            constexpr Vec2(float x, float y) : x(x), y(y) {}

            // Generic constructor from any 2-component vector (e.g. glm::vec2, custom struct with .x, .y)
            template <typename T, typename = std::enable_if_t<!std::is_same_v<std::decay_t<T>, Vec2>>>
            constexpr Vec2(const T& v) : x(static_cast<float>(v.x)), y(static_cast<float>(v.y)) {}
        };

        struct Vec4 {
            float x{0.0f};
            float y{0.0f};
            float z{0.0f};
            float w{1.0f};

            Vec4() = default;
            constexpr explicit Vec4(float scalar) : x(scalar), y(scalar), z(scalar), w(scalar) {}
            constexpr Vec4(float x, float y, float z = 0.0f, float w = 1.0f) : x(x), y(y), z(z), w(w) {}

            // Generic constructor from any 4-component vector (e.g. glm::vec4, custom struct with .x, .y, .z, .w)
            template <typename T, typename = std::enable_if_t<!std::is_same_v<std::decay_t<T>, Vec4>>>
            constexpr Vec4(const T& v) : x(static_cast<float>(v.x)), y(static_cast<float>(v.y)), z(static_cast<float>(v.z)), w(static_cast<float>(v.w)) {}
        };
    }
}

struct Rect {
    float x{0.0f};
    float y{0.0f};
    float width{0.0f};
    float height{0.0f};
};

struct UIHierarchyComponent {
    EntityID parent = NULL_ENTITY;
    std::vector<EntityID> children;
};

struct UIRectTransform {
    Bett::detail::Vec2 position{0.0f, 0.0f};
    Bett::detail::Vec2 size{100.0f, 30.0f};
    Bett::detail::Vec2 anchor{0.0f, 0.0f};
    Rect computedBounds{0.0f, 0.0f, 0.0f, 0.0f};
};

struct UIInteractableComponent {
    bool isHovered{false};
    bool isPressed{false};
    bool isFocused{false};
    std::function<void(EntityID)> onClick;
    std::function<void(EntityID)> onHoverEnter;
    std::function<void(EntityID)> onHoverExit;
};

struct UIRenderComponent {
    Bett::detail::Vec4 normalColor{0.2f, 0.2f, 0.2f, 1.0f};
    Bett::detail::Vec4 hoverColor{0.35f, 0.35f, 0.35f, 1.0f};
    Bett::detail::Vec4 activeColor{0.1f, 0.5f, 0.8f, 1.0f};
    Bett::detail::Vec4 radius{0.f, 0.f, 0.f, 0.f}; // x: Top Left, y: Top Right, z: Bottom Right, w: Bottom Left
    Bett::detail::Vec4 border{0.f, 0.f, 0.f, 0.f}; // x: Top, y: Right, z: Bottom, w: Left (border thickness)
    Bett::detail::Vec4 borderColor{0.f, 0.f, 0.f, 1.f};
    Bett::detail::Vec4 borderHoverColor{0.f, 0.f, 0.f, 1.f};
    Bett::detail::Vec4 borderActiveColor{0.f, 0.f, 0.f, 1.f};
    int zOrder{0};

    void SetBorder(float width) {
        border = Bett::detail::Vec4(width, width, width, width);
    }

    void SetBorder(float top, float right, float bottom, float left) {
        border = Bett::detail::Vec4(top, right, bottom, left);
    }

    void SetBorderColor(const Bett::detail::Vec4& color) {
        borderColor = color;
        borderHoverColor = color;
        borderActiveColor = color;
    }
};

struct UIImageComponent {
    uint32_t textureID{0};
    Bett::detail::Vec2 uvMin{0.0f, 0.0f};
    Bett::detail::Vec2 uvMax{1.0f, 1.0f};
    Bett::detail::Vec4 tint{1.0f, 1.0f, 1.0f, 1.0f};
};

struct UITextComponent {
    std::string text;
    float fontSize{16.0f};
    Bett::detail::Vec4 color{1.0f, 1.0f, 1.0f, 1.0f};
};

class CBettUISystem {
public:
    using LoggerAPI = void (*)(const std::string&);
    using Vec2 = Bett::detail::Vec2;
    using Vec4 = Bett::detail::Vec4;

private:
    CBettECS* ecs{nullptr};
    LoggerAPI debugAPI{nullptr};
    LoggerAPI warningAPI{nullptr};
    LoggerAPI errorAPI{nullptr};

    EntityID activeHoveredEntity = NULL_ENTITY;
    EntityID activePressedEntity = NULL_ENTITY;

    std::vector<std::function<void()>> customUIFunction;

public:
    explicit CBettUISystem(CBettECS* ecsInstance = nullptr) : ecs(ecsInstance) {}

    void SetECS(CBettECS* ecsInstance) { ecs = ecsInstance; }

    void AttachDebugAPI(LoggerAPI debugApiFunc) {
        debugAPI = debugApiFunc;
    }

    void AttachWarningAPI(LoggerAPI warningApiFunc) {
        warningAPI = warningApiFunc;
    }

    void AttachErrorAPI(LoggerAPI errorApiFunc) {
        errorAPI = errorApiFunc;
    }

    // Register a custom UI function (e.g. scroll, input, slider, colour picker)
    void AddCustomUIFunction(std::function<void()> func) {
        if (func) {
            customUIFunction.push_back(std::move(func));
        }
    }

    void ClearCustomUIFunctions() {
        customUIFunction.clear();
    }

    // Helper to spawn a GameObject pre-configured with core UI components
    GameObject CreateUIElement() {
        if (!ecs) {
            BettBLogError("CBettUISystem: No ECS attached!");
            return GameObject(NULL_ENTITY, *ecs);
        }
        
        GameObject gm = ecs->CreateGameObject();
        gm.AddComponent<UIRectTransform>();
        gm.AddComponent<UIHierarchyComponent>();
        gm.AddComponent<UIRenderComponent>();
        return gm;
    }

    // Checks if a 2D point is inside a rectangle, taking corner radius into account
    static bool ContainsPoint(const Rect& rect, const Vec4& radius, const Vec2& point) {
        // Quick reject if outside the main bounding box
        if (point.x < rect.x || point.x > (rect.x + rect.width) ||
            point.y < rect.y || point.y > (rect.y + rect.height)) {
            return false;
        }

        float halfW = rect.width * 0.5f;
        float halfH = rect.height * 0.5f;
        float maxR = std::min(halfW, halfH);

        // Position relative to rect center
        float cx = rect.x + halfW;
        float cy = rect.y + halfH;
        float px = point.x - cx;
        float py = point.y - cy;

        // Pick radius for the current corner quadrant
        // Top-Left: (px < 0, py < 0) -> radius.x
        // Top-Right: (px >= 0, py < 0) -> radius.y
        // Bottom-Right: (px >= 0, py >= 0) -> radius.z
        // Bottom-Left: (px < 0, py >= 0) -> radius.w
        float r = 0.0f;
        if (px < 0.0f && py < 0.0f) {
            r = radius.x;
        } else if (px >= 0.0f && py < 0.0f) {
            r = radius.y;
        } else if (px >= 0.0f && py >= 0.0f) {
            r = radius.z;
        } else {
            r = radius.w;
        }

        // Clamp radius so it doesn't overshoot half size
        r = std::min(r, maxR);
        if (r <= 0.0f) {
            return true; // No rounding on this corner
        }

        // If point is in the corner box, check circular distance to corner arc center
        float qx = std::abs(px) - halfW + r;
        float qy = std::abs(py) - halfH + r;

        if (qx > 0.0f && qy > 0.0f) {
            return (qx * qx + qy * qy) <= (r * r);
        }

        return true;
    }

    // Checks if a 2D point lies within the border area of a rounded rectangle
    static bool IsPointOnBorder(const Rect& rect, const Vec4& radius, const Vec4& border, const Vec2& point) {
        if (!ContainsPoint(rect, radius, point)) {
            return false;
        }

        // Inset rectangle by border thickness
        Rect innerRect{
            rect.x + border.w, // Left
            rect.y + border.x, // Top
            std::max(0.0f, rect.width - border.w - border.y),
            std::max(0.0f, rect.height - border.x - border.z)
        };

        // If border thickness covers the entire element, any point inside is on the border
        if (innerRect.width <= 0.0f || innerRect.height <= 0.0f) {
            return true;
        }

        Vec4 innerRadius{
            std::max(0.0f, radius.x - std::max(border.x, border.w)),
            std::max(0.0f, radius.y - std::max(border.x, border.y)),
            std::max(0.0f, radius.z - std::max(border.z, border.y)),
            std::max(0.0f, radius.w - std::max(border.z, border.w))
        };

        return !ContainsPoint(innerRect, innerRadius, point);
    }

    // Update layout and handle mouse hover/click interaction
    void Update(const Vec2& mousePos, bool mouseButtonDown, float screenWidth, float screenHeight) {
        if (!ecs) {
            BettBLogError("CBettUISystem: No ECS attached!");
            return;
        }

        UpdateLayout(screenWidth, screenHeight);
        UpdateInteraction(mousePos, mouseButtonDown);

        // Custom function updates for different UI elements like scroll, input, slider, colour picker and more
        for (auto& func : customUIFunction) {
            func();
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

private:
    // Calculate bounding rects for all transforms
    void UpdateLayout(float screenWidth, float screenHeight) {
        if (!ecs) return;

        auto& transforms = ecs->Store<UIRectTransform>().All();
        for (auto& transform : transforms) {
            transform.computedBounds.x = transform.position.x;
            transform.computedBounds.y = transform.position.y;
            transform.computedBounds.width = transform.size.x;
            transform.computedBounds.height = transform.size.y;
        }
    }

    // Hit test interactable entities and fire hover / click events
    void UpdateInteraction(const Vec2& mousePos, bool mouseButtonDown) {
        if (!ecs) return;

        auto& interactables = ecs->Store<UIInteractableComponent>();
        auto& entities = interactables.Entities();

        EntityID topHovered = NULL_ENTITY;
        int highestZ = INT32_MIN;

        // Find topmost hovered UI element
        for (EntityID id : entities) {
            if (!ecs->Has<UIRectTransform>(id)) continue; // Only consider entities with a rect transform

            const auto& rect = ecs->GetComponent<UIRectTransform>(id).computedBounds;
            Vec4 radius{0.0f, 0.0f, 0.0f, 0.0f};
            int zOrder = 0;

            if (ecs->Has<UIRenderComponent>(id)) {
                const auto& render = ecs->GetComponent<UIRenderComponent>(id);
                radius = render.radius;
                zOrder = render.zOrder;
            }

            if (ContainsPoint(rect, radius, mousePos)) {
                // Select as topmost if none found yet, or if it has an equal or higher z-order
                if (topHovered == NULL_ENTITY || zOrder >= highestZ) {
                    topHovered = id;
                    highestZ = zOrder;
                }
            }
        }

        // Handle hover state transitions when the topmost hovered element changes
        if (topHovered != activeHoveredEntity) {
            // Clear hover state on previously hovered entity and fire onHoverExit
            if (activeHoveredEntity != NULL_ENTITY && ecs->Has<UIInteractableComponent>(activeHoveredEntity)) {
                auto& prev = ecs->GetComponent<UIInteractableComponent>(activeHoveredEntity);
                prev.isHovered = false;
                if (prev.onHoverExit) prev.onHoverExit(activeHoveredEntity);
            }
            
            // Set hover state on newly hovered entity and fire onHoverEnter
            if (topHovered != NULL_ENTITY) {
                auto& next = ecs->GetComponent<UIInteractableComponent>(topHovered);
                next.isHovered = true;
                if (next.onHoverEnter) next.onHoverEnter(topHovered);
            }
            activeHoveredEntity = topHovered;
        }

        // Handle press and click release lifecycle
        if (mouseButtonDown) {
            if (activePressedEntity == NULL_ENTITY && topHovered != NULL_ENTITY) {
                activePressedEntity = topHovered;
                ecs->GetComponent<UIInteractableComponent>(topHovered).isPressed = true;
            }
        } else {
            if (activePressedEntity != NULL_ENTITY) {
                if (ecs->Has<UIInteractableComponent>(activePressedEntity)) {
                    auto& comp = ecs->GetComponent<UIInteractableComponent>(activePressedEntity);
                    comp.isPressed = false;
                    // Only fire onClick if mouse was released over the same element it pressed
                    if (activePressedEntity == topHovered && comp.onClick) {
                        comp.onClick(activePressedEntity);
                    }
                }
                activePressedEntity = NULL_ENTITY;
            }
        }
    }
};

#endif
