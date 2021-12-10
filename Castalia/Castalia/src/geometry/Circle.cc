#include <src/geometry/Circle.h>
#include <src/geometry/Line.h>
#include <src/geometry/Vector.h>

using namespace geometry;

Circle::Circle() :
        _center(Point()), _radius(0) {
}

Circle::Circle(const Point &center, double radius) :
        _center(center), _radius(radius) {
}

Circle::Circle(const Point &a, const Point &b, const Point &c) {
    Vector ab = b - a;
    Vector bc = c - b;

    if (ab.x() * bc.y() - ab.y() * bc.x() == 0) { // all points on a line
        return;
    }

    LineIntersectionSolutionSet solutionSet =
            Line::forNormal(a.midPoint(b), ab).intersection(
                    Line::forNormal(b.midPoint(c), bc));
    if (!solutionSet.isOne()) {
        return;
    }

    _center = solutionSet.point();
    _radius = (_center - a).length();
}

const Point &Circle::center() const {
    return _center;
}

double Circle::radius() const {
    return _radius;
}

bool Circle::contains(const Point &p) const {
    return (_center - p).squaredLength() <= _radius * _radius;
}

Point Circle::top() const {
    return Point(_center.x(), _center.y() - _radius);
}

Point Circle::bottom() const {
    return Point(_center.x(), _center.y() + _radius);
}

Point Circle::left() const {
    return Point(_center.x() - _radius, _center.y());
}

Point Circle::right() const {
    return Point(_center.x() + _radius, _center.y());
}
