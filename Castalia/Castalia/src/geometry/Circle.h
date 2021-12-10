#pragma once

#include <Point.h>

namespace geometry {

class Circle {
public:
    Circle();

    Circle(const Point &center, double radius);

    Circle(const Point &a, const Point &b, const Point &c);

    const Point &center() const;

    double radius() const;

    bool contains(const Point &p) const;

    Point top() const;

    Point bottom() const;

    Point left() const;

    Point right() const;

protected:
    Point _center;
    double _radius;
};

} //end namespace geometry
