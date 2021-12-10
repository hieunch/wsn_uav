#include <Vector.h>
#include <cmath>

using namespace geometry;

Vector geometry::operator*(double factor, const Vector &v) {
    return v * factor;
}

Vector::Vector() :
        _x(0), _y(0) {
}

Vector::Vector(double x, double y) :
        _x(x), _y(y) {
}

Vector Vector::operator-() const {
    return Vector(-_x, -_y);
}

Vector &Vector::operator+=(const Vector &v) {
    _x += v._x;
    _y += v._y;

    return *this;
}

Vector &Vector::operator-=(const Vector &v) {
    _x -= v._x;
    _y -= v._y;

    return *this;
}

Vector &Vector::operator*=(double factor) {
    _x *= factor;
    _y *= factor;

    return *this;
}

Vector &Vector::operator/=(double factor) {
    _x /= factor;
    _y /= factor;

    return *this;
}

Vector Vector::operator+(const Vector &v) const {
    return Vector(_x + v._x, _y + v._y);
}

Vector Vector::operator-(const Vector &v) const {
    return Vector(_x - v._x, _y - v._y);
}

Vector Vector::operator*(double factor) const {
    return Vector(_x * factor, _y * factor);
}

Vector Vector::operator/(double factor) const {
    return Vector(_x / factor, _y / factor);
}

bool Vector::operator==(const Vector &v) const {
    return _x == v._x && _y == v._y;
}

bool Vector::operator!=(const Vector &v) const {
    return _x != v._x || _y != v._y;
}

double Vector::dotProduct(const Vector &v) const {
    return _x * v._x + _y * v._y;
}

double Vector::length() const {
    return std::sqrt(squaredLength());
}

double Vector::squaredLength() const {
    return _x * _x + _y * _y;
}

Vector Vector::perpendicular() const {
    return Vector(-_y, _x);
}

bool Vector::isParallelTo(const Vector &v) const {
    return dotProduct(v) == 0;
}

double Vector::x() const {
    return _x;
}

double Vector::y() const {
    return _y;
}
