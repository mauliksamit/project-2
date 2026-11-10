#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "MathUtil.h"
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

namespace ECSEngine {

class WindowManager {
public:
    WindowManager(unsigned int width, unsigned int height, const std::string& title);
    ~WindowManager() = default;

    // shared_ptr, so the window lives as long as someone holds this
    std::shared_ptr<sf::RenderWindow> GetWindow() const;
    Rect GetWindowBounds() const;

    // puts worldPt at screenPt, keeps the scale
    void SetCamera(const Point2D& worldPt, const Point2D& screenPt);
    void SetWorldCenter(const Point2D& worldCenter);
    // scale comes from the rect's width, so use the same shape as the window
    void SetWorldRect(const Rect& worldCameraRect);
    void SetWorldScale(float worldUnitsPerPixel);
    Point2D GetWorldCenter() const;
    float GetWorldScale() const;

    // float versions convert lengths, not positions
    float WindowToWorld(float x) const;
    float WorldToWindow(float x) const;
    Rect WindowToWorld(const Rect& rect) const;
    Rect WorldToWindow(const Rect& rect) const;
    Point2D WorldToWindow(const Point2D& pt) const;
    Point2D WindowToWorld(const Point2D& pt) const;

    // r is in window coordinates, gets drawn on the next Display()
    void DebugDrawRect(const Rect& r, int red, int green, int blue, int alpha = 255);
    void DebugFrameRect(const Rect& r, int red, int green, int blue, int alpha = 255);

    // added to the WindowManager API so the engine can draw the debug rects
    // on top of the sprites. engine calls it after the systems, systems shouldn't call display()
    void Display();

private:
    std::shared_ptr<sf::RenderWindow> mWindow;
    Point2D mWorldCenter;       // world point at the center of the window
    float mWorldUnitsPerPixel;  // How many world units per pixel
    std::vector<sf::RectangleShape> mDebugShapes;
};

}  // namespace ECSEngine
