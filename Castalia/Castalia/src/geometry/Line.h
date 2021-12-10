#pragma once

#include <Point.h>
#include <Vector.h>

namespace geometry {

class LineIntersectionSolutionSet;

class Line {
public:
    enum Type {
        NULL_LINE, LINE, RAY, SEGMENT
    };

    Line();

    static Line forDirection(const Point &supportVector,
            const Vector &direction);

    static Line forNormal(const Point &supportVector, const Vector &normal);

    static Line ray(const Point &supportVector, const Vector &direction);

    static Line segment(const Point &point1, const Point &point2);

    const Point &supportVector() const;

    const Point &startPoint() const;

    const Vector &direction() const;

    const Point &endPoint() const;

    Type type() const;

    bool isNull() const;

    bool isLine() const;

    bool isRay() const;

    bool isSegment() const;

    Line asLine() const;

    void setStartPoint(const Point &point);

    void setEndPoint(const Point &point);

    void setDirection(const Vector &direction);

    void invertDirection();

    bool addPoint(const Point &point);

    LineIntersectionSolutionSet intersection(const Line &line) const;

    Vector normal() const;

    Line perpendicular(const Point &point) const;

    bool isParallelTo(const Line &line) const;

    bool contains(const Point &p) const;

    bool overlaps(const Line &line) const;

    bool sameSide(const Point &p1, const Point &p2) const;

protected:
    Type _type;
    Point _startPoint;
    Point _endPoint;
    Vector _direction;

    Line(Type type, const Point &start, const Point &end,
            const Vector &direction);

    bool lineContains(const Point &p) const;

    bool intersectionCoefficient(const Line &line, double &coefficient) const;

    LineIntersectionSolutionSet lineIntersection(const Line &line) const;

    bool containsCoefficient(double coefficient) const;

    double coefficientForPointOnLine(const Point &p) const;
};

class LineIntersectionSolutionSet {
public:
    enum Type {
        NO_SOLUTION, ONE_SOLUTION, INFINITE_SOLUTIONS
    };

    LineIntersectionSolutionSet();

    LineIntersectionSolutionSet(const Point &point);

    static LineIntersectionSolutionSet noSolution();

    static LineIntersectionSolutionSet infiniteSolutions();

    bool isEmpty() const;

    bool isOne() const;

    bool isInfinite() const;

    Type type() const;

    const Point &point() const;

private:
    Point _point;
    Type _type;
};

} //end namespace geometry
