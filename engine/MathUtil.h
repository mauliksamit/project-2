#ifndef MATHUTIL_H
#define MATHUTIL_H

#include <algorithm>
#include <cmath>
#include <iostream>

namespace ECSEngine {

inline constexpr float kPi = 3.14159265358979f;

// Plain 2D vector used for positions, velocities and sizes.
// Note: mixing int and float arguments (Point2D(1, 2.0f)) is ambiguous, write float literals.
struct Point2D {
    float x, y;
    Point2D(float x = 0, float y = 0) : x(x), y(y) {}
    Point2D(int x, int y) : x(static_cast<float>(x)), y(static_cast<float>(y)) {}

    Point2D& operator+=(const Point2D& rhs) {
        x += rhs.x;
        y += rhs.y;
        return *this;
    }
    Point2D& operator-=(const Point2D& rhs) {
        x -= rhs.x;
        y -= rhs.y;
        return *this;
    }
    Point2D& operator*=(float s) {
        x *= s;
        y *= s;
        return *this;
    }
    float Length() const { return std::sqrt(x * x + y * y); }
};

inline Point2D operator+(Point2D lhs, const Point2D& rhs) { return lhs += rhs; }
inline Point2D operator-(Point2D lhs, const Point2D& rhs) { return lhs -= rhs; }
inline Point2D operator-(const Point2D& p) { return Point2D(-p.x, -p.y); }
inline Point2D operator*(Point2D lhs, float s) { return lhs *= s; }
inline Point2D operator*(float s, Point2D rhs) { return rhs *= s; }
inline Point2D operator/(const Point2D& lhs, float s) { return Point2D(lhs.x / s, lhs.y / s); }
inline bool operator==(const Point2D& a, const Point2D& b) { return a.x == b.x && a.y == b.y; }
inline bool operator!=(const Point2D& a, const Point2D& b) { return !(a == b); }

inline std::ostream& operator<<(std::ostream& os, const Point2D& p) {
    os << "(" << p.x << ", " << p.y << ")";
    return os;
}

struct Line {
    Point2D p1, p2;

    Line(Point2D p1 = {0, 0}, Point2D p2 = {0, 0}) : p1(p1), p2(p2) {}
    Line(float x1, float y1, float x2, float y2) : p1(x1, y1), p2(x2, y2) {}
};

inline std::ostream& operator<<(std::ostream& os, const Line& l) {
    os << "L{" << l.p1 << ", " << l.p2 << "}";
    return os;
}

struct Circle {
    Point2D center;
    float radius;

    Circle(Point2D c = {0, 0}, float r = 0) : center(c), radius(r) {}
    Circle(float x, float y, float r) : center(x, y), radius(r) {}
};

// Axis-aligned rectangle. y grows downwards (screen convention), so Top() < Bottom().
struct Rect {
    Point2D topLeft;
    float width = 0;
    float height = 0;

    Rect() = default;
    Rect(const Point2D& topLeft, float width, float height)
        : topLeft(topLeft), width(width), height(height) {}
    Rect(float x, float y, float width, float height)
        : topLeft(x, y), width(width), height(height) {}

    float Left() const { return topLeft.x; }
    float Right() const { return topLeft.x + width; }
    float Top() const { return topLeft.y; }
    float Bottom() const { return topLeft.y + height; }
    Point2D Center() const { return Point2D(topLeft.x + width / 2, topLeft.y + height / 2); }

    // Same size, moved by offset. Used to turn a box relative to an entity into world space.
    Rect Translated(const Point2D& offset) const { return Rect(topLeft + offset, width, height); }

    bool Contains(const Point2D& p) const {
        return p.x >= Left() && p.x < Right() && p.y >= Top() && p.y < Bottom();
    }
};

inline std::ostream& operator<<(std::ostream& os, const Rect& r) {
    os << "R{" << r.topLeft << ", " << r.width << "x" << r.height << "}";
    return os;
}

// True if the rectangles overlap by a positive area. Touching edges do not count,
// so an entity resting exactly on the ground is not "inside" it.
inline bool Intersects(const Rect& a, const Rect& b) {
    return a.Left() < b.Right() && b.Left() < a.Right() && a.Top() < b.Bottom() &&
           b.Top() < a.Bottom();
}

// Penetration depth on each axis. Only meaningful when Intersects(a, b) is true.
inline Point2D Overlap(const Rect& a, const Rect& b) {
    return Point2D(std::min(a.Right(), b.Right()) - std::max(a.Left(), b.Left()),
                   std::min(a.Bottom(), b.Bottom()) - std::max(a.Top(), b.Top()));
}

inline float Sign(float v) { return static_cast<float>((v > 0) - (v < 0)); }

inline float Lerp(float a, float b, float t) { return a + (b - a) * t; }

// Moves current towards target by at most maxDelta (handy for acceleration up to a max speed).
inline float MoveTowards(float current, float target, float maxDelta) {
    if (std::fabs(target - current) <= maxDelta) {
        return target;
    }
    return current + Sign(target - current) * maxDelta;
}

}  // namespace ECSEngine

#endif  // MATHUTIL_H
