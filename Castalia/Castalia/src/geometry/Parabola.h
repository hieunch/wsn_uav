#pragma once

#include <Point.h>

namespace geometry {

class ParabolaIntersectionSolutionSet;

class Parabola {
public:
    Parabola();

    Parabola(const Point &focus, double baselineY);

    const Point &focus() const;

    double baselineY() const;

    double operator()(double x) const;

    Point at(double x) const;

    const Point &vertex() const;

    double stretchFactor() const;

    // parabola formula: (x-h)^2 = 4a(y-k) with vertex=(h,k) and a=dist(line, vertex)/2
    double a() const;

    double h() const;

    double k() const;

    ParabolaIntersectionSolutionSet intersection(
            const Parabola &parabola) const;

protected:
    Point _focus;
    double _baselineY;

    double _stretchFactor;
    Point _vertex;
};

class ParabolaIntersectionSolutionSet {
public:
    enum Type {
        NO_SOLUTION, ONE_SOLUTION, TWO_SOLUTIONS, INFINITE_SOLUTIONS
    };

    ParabolaIntersectionSolutionSet();

    ParabolaIntersectionSolutionSet(const Point &point);

    ParabolaIntersectionSolutionSet(const Point &point1, const Point &point2);

    static ParabolaIntersectionSolutionSet noSolution();

    static ParabolaIntersectionSolutionSet infiniteSolutions();

    bool isEmpty() const;

    bool isOne() const;

    bool isTwo() const;

    bool isInfinite() const;

    const Type &type() const;

    const Point &point() const;

    const Point &leftPoint() const;

    const Point &rightPoint() const;

private:
    Type _type;
    Point _leftPoint;
    Point _rightPoint;
};

} //end namespace geometry
