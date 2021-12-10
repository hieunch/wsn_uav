#include <Point.h>

using namespace geometry;

Point geometry::operator*(double factor, const Point &p) {
    return p * factor;
}

Point::Point() :
        _x(0), _y(0) {
}

Point::Point(double x, double y) :
        _x(x), _y(y) {
}

Point Point::operator-() const {
    return Point(-_x, -_y);
}

Point &Point::operator+=(const Vector &v) {
    _x += v.x();
    _y += v.y();

    return *this;
}

Point &Point::operator-=(const Vector &v) {
    _x -= v.x();
    _y -= v.y();

    return *this;
}

Point &Point::operator*=(double factor) {
    _x *= factor;
    _y *= factor;

    return *this;
}

Point &Point::operator/=(double factor) {
    _x /= factor;
    _y /= factor;

    return *this;
}

Point Point::operator+(const Vector &v) const {
    return Point(_x + v.x(), _y + v.y());
}

Point Point::operator-(const Vector &v) const {
    return Point(_x - v.x(), _y - v.y());
}

Vector Point::operator-(const Point &p) const {
    return Vector(_x - p._x, _y - p._y);
}

Point Point::operator*(double factor) const {
    return Point(_x * factor, _y * factor);
}

Point Point::operator/(double factor) const {
    return Point(_x / factor, _y / factor);
}

bool Point::operator==(const Point &p) const {
    return _x == p._x && _y == p._y;
}

bool Point::operator!=(const Point &p) const {
    return _x != p._x || _y != p._y;
}

bool Point::operator<(const Point &p) const {
    return _x < p._x || (_x == p._x && _y < p._y);
}

bool Point::operator<=(const Point &p) const {
    return *this == p || *this < p;
}

bool Point::operator>(const Point &p) const {
    return !(*this <= p);
}

bool Point::operator>=(const Point &p) const {
    return *this == p || *this > p;
}

double Point::x() const {
    return _x;
}

double Point::y() const {
    return _y;
}

Vector Point::toVector() const {
    return Vector(_x, _y);
}

Point Point::midPoint(const Point &p) const {
    return Point((_x + p._x) / 2, (_y + p._y) / 2);
}

bool geometry::clockwise(const Point &a, const Point &b, const Point &c) {
    Vector ab = b - a;
    Vector bc = c - b;

    return ab.x() * bc.y() - ab.y() * bc.x() < 0;
}

bool geometry::pointsOnLine(const Point &a, const Point &b, const Point &c) {
    Vector ab = b - a;
    Vector bc = c - b;

    return ab.x() * bc.y() - ab.y() * bc.x() == 0;
}
