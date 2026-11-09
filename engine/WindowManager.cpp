#include "WindowManager.h"

#include <cstdint>

namespace ECSEngine {

static sf::Color MakeColor(int red, int green, int blue, int alpha) {
    return sf::Color(static_cast<std::uint8_t>(red), static_cast<std::uint8_t>(green),
                     static_cast<std::uint8_t>(blue), static_cast<std::uint8_t>(alpha));
}

static sf::RectangleShape MakeDebugShape(const Rect& r) {
    sf::RectangleShape shape({r.width, r.height});
    shape.setPosition({r.topLeft.x, r.topLeft.y});
    return shape;
}

// world starts lined up with the window, window can't be resized so the math stays simple
WindowManager::WindowManager(unsigned int width, unsigned int height, const std::string& title)
    : mWindow(std::make_shared<sf::RenderWindow>(sf::VideoMode({width, height}), title,
                                                 sf::Style::Titlebar | sf::Style::Close)),
      mWorldCenter(width / 2.0f, height / 2.0f),
      mWorldUnitsPerPixel(1.0f) {}

std::shared_ptr<sf::RenderWindow> WindowManager::GetWindow() const { return mWindow; }

Rect WindowManager::GetWindowBounds() const {
    sf::Vector2u windowSize = mWindow->getSize();
    return Rect{Point2D(0.0f, 0.0f), static_cast<float>(windowSize.x),
                static_cast<float>(windowSize.y)};
}

void WindowManager::DebugDrawRect(const Rect& r, int red, int green, int blue, int alpha) {
    sf::RectangleShape shape = MakeDebugShape(r);
    shape.setFillColor(MakeColor(red, green, blue, alpha));
    mDebugShapes.push_back(shape);
}

void WindowManager::DebugFrameRect(const Rect& r, int red, int green, int blue, int alpha) {
    sf::RectangleShape shape = MakeDebugShape(r);
    shape.setFillColor(sf::Color::Transparent);
    shape.setOutlineColor(MakeColor(red, green, blue, alpha));
    shape.setOutlineThickness(1.0f);
    mDebugShapes.push_back(shape);
}

void WindowManager::Display() {
    for (const sf::RectangleShape& shape : mDebugShapes) {
        mWindow->draw(shape);
    }
    mDebugShapes.clear();
    mWindow->display();
}

Rect WindowManager::WindowToWorld(const Rect& rect) const {
    return Rect{WindowToWorld(rect.topLeft), WindowToWorld(rect.width), WindowToWorld(rect.height)};
}

Point2D WindowManager::WindowToWorld(const Point2D& pt) const {
    Rect windowBounds = GetWindowBounds();
    float worldX = (pt.x - windowBounds.width / 2.0f) * mWorldUnitsPerPixel + mWorldCenter.x;
    float worldY = (pt.y - windowBounds.height / 2.0f) * mWorldUnitsPerPixel + mWorldCenter.y;
    return Point2D(worldX, worldY);
}

Rect WindowManager::WorldToWindow(const Rect& rect) const {
    return Rect{WorldToWindow(rect.topLeft), WorldToWindow(rect.width), WorldToWindow(rect.height)};
}

Point2D WindowManager::WorldToWindow(const Point2D& pt) const {
    Rect windowBounds = GetWindowBounds();
    float windowX = (pt.x - mWorldCenter.x) / mWorldUnitsPerPixel + windowBounds.width / 2.0f;
    float windowY = (pt.y - mWorldCenter.y) / mWorldUnitsPerPixel + windowBounds.height / 2.0f;
    return Point2D(windowX, windowY);
}

void WindowManager::SetCamera(const Point2D& worldPt, const Point2D& screenPt) {
    Rect windowBounds = GetWindowBounds();
    float centerX = worldPt.x - (screenPt.x - windowBounds.width / 2.0f) * mWorldUnitsPerPixel;
    float centerY = worldPt.y - (screenPt.y - windowBounds.height / 2.0f) * mWorldUnitsPerPixel;
    mWorldCenter = Point2D(centerX, centerY);
}

Point2D WindowManager::GetWorldCenter() const { return mWorldCenter; }

float WindowManager::GetWorldScale() const { return mWorldUnitsPerPixel; }

void WindowManager::SetWorldCenter(const Point2D& worldCenter) { mWorldCenter = worldCenter; }

void WindowManager::SetWorldScale(float worldUnitsPerPixel) {
    mWorldUnitsPerPixel = worldUnitsPerPixel;
}

void WindowManager::SetWorldRect(const Rect& worldCameraRect) {
    Rect windowBounds = GetWindowBounds();
    mWorldUnitsPerPixel = worldCameraRect.width / windowBounds.width;

    float centerX = worldCameraRect.topLeft.x + worldCameraRect.width / 2.0f;
    float centerY = worldCameraRect.topLeft.y + worldCameraRect.height / 2.0f;
    mWorldCenter = Point2D(centerX, centerY);
}

float WindowManager::WindowToWorld(float x) const { return x * mWorldUnitsPerPixel; }

float WindowManager::WorldToWindow(float x) const { return x / mWorldUnitsPerPixel; }

}  // namespace ECSEngine
