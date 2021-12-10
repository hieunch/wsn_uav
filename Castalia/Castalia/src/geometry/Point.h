#pragma once
#include <Vector.h>

namespace geometry {

class Point {
public:
    Point();

    Point(double x, double y);

    double x() const;

    double y() const;

    bool operator==(const Point &p) const;

    bool operator!=(const Point &p) const;

    bool operator<(const Point &p) const;

    bool operator<=(const Point &p) const;

    bool operator>(const Point &p) const;

    bool operator>=(const Point &p) const;

    Point operator-() const;

    Point &operator+=(const Vector &p);

    Point &operator-=(const Vector &v);

    Point &operator*=(double factor);

    Point &operator/=(double factor);

    Point operator+(const Vector &p) const;

    Vector operator-(const Point &p) const;

    Point operator-(const Vector &v) const;

    Point operator*(double factor) const;

    Point operator/(double factor) const;

    Vector toVector() const;

    Point midPoint(const Point &p) const;

private:
    double _x;
    double _y;
};

Point operator*(double factor, const Point &p);

bool clockwise(const Point &a, const Point &b, const Point &c);

bool pointsOnLine(const Point &a, const Point &b, const Point &c);

} //end namespace geometry
