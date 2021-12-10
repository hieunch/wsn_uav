#pragma once

namespace geometry {

class Point;

class Vector {
public:
    Vector();

    Vector(double x, double y);

    double x() const;

    double y() const;

    bool operator==(const Vector &v) const;

    bool operator!=(const Vector &v) const;

    Vector operator-() const;

    Vector &operator+=(const Vector &v);

    Vector &operator-=(const Vector &v);

    Vector &operator*=(double factor);

    Vector &operator/=(double factor);

    Vector operator+(const Vector &v) const;

    Vector operator-(const Vector &v) const;

    Vector operator*(double factor) const;

    Vector operator/(double factor) const;

    double dotProduct(const Vector &v) const;

    double length() const;

    double squaredLength() const;

    bool isParallelTo(const Vector &v) const;

    Vector perpendicular() const;

private:
    double _x;
    double _y;
};

Vector operator*(double factor, const Vector &v);

} //end namespace geometry
