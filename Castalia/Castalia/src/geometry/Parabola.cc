#include <Parabola.h>
#include <algorithm>
#include <cmath>

using namespace geometry;

Parabola::Parabola() {
}

Parabola::Parabola(const Point &focus, double baselineY) :
        _focus(focus), _baselineY(baselineY) {
    _stretchFactor = (_focus.y() - _baselineY) / 2;
    _vertex = Point(_focus.x(), _focus.y() - _stretchFactor);
}

const Point &Parabola::focus() const {
    return _focus;
}

double Parabola::baselineY() const {
    return _baselineY;
}

double Parabola::stretchFactor() const {
    return _stretchFactor;
}

const Point &Parabola::vertex() const {
    return _vertex;
}

double Parabola::a() const {
    return _stretchFactor;
}

double Parabola::h() const {
    return _vertex.x();
}

double Parabola::k() const {
    return _vertex.y();
}

double Parabola::operator()(double x) const {
    // parabola formula: (x-h)^2 = 4a(y-k) with vertex=(h,k) and a=dist(line, vertex)/2
    return (x - h()) * (x - h()) / (4 * a()) + k();
}

Point Parabola::at(double x) const {
    return Point(x, (*this)(x));
}

ParabolaIntersectionSolutionSet Parabola::intersection(
        const Parabola &parabola) const {
    double h1 = h();
    double k1 = k();
    double a1 = a();

    double h2 = parabola.h();
    double k2 = parabola.k();
    double a2 = parabola.a();

    double x;

    if (a1 == 0 && a2 == 0) { // no intersection
        return ParabolaIntersectionSolutionSet::noSolution();
    }

    if (a1 == a2) { // parabolas have same stretch factor
        x = (h1 + h2) / 2;
    } else if (a1 == 0) { // first parabola lies on baseline -> degenerates to point
        x = h1;
        return ParabolaIntersectionSolutionSet(parabola.at(x));
    } else if (a2 == 0) { // second parabola lies on baseline -> degenerates to point
        x = h2;
    } else { // two intersection points
        /*double p = 2*(h2*a1-h1*a2)/(a2-a1);
         double q = (4*a1*a2*(k1-k2)+h1*h1*a2-h2*h2*a1)/(a2-a1);
         double D = p*p/4-q;

         double sqrtD = sqrt(D);

         x = -p/2-sqrtD;
         double x2 = -p/2+sqrtD;

         return ParabolaIntersectionSolutionSet(at(x), at(x2));*/

        double A = a2 - a1;
        double B = 2 * (a1 * h2 - a2 * h1);
        double C = a2 * (h1 * h1 + 4 * a1 * k1) - a1 * (h2 * h2 + 4 * a2 * k2);

        double q =
                B > 0 ? (B + std::sqrt(B * B - 4 * A * C)) / -2 : (-B
                        + std::sqrt(B * B - 4 * A * C)) / 2;

        x = q / A;
        double x2 = C / q;

        return ParabolaIntersectionSolutionSet(at(x), at(x2));
    }

    return ParabolaIntersectionSolutionSet(at(x));
}

ParabolaIntersectionSolutionSet::ParabolaIntersectionSolutionSet() :
        _type(NO_SOLUTION) {
}

ParabolaIntersectionSolutionSet::ParabolaIntersectionSolutionSet(
        const Point &point) :
        _type(ONE_SOLUTION), _leftPoint(point), _rightPoint(point) {
}

ParabolaIntersectionSolutionSet::ParabolaIntersectionSolutionSet(
        const Point &point1, const Point &point2) :
        _type(TWO_SOLUTIONS), _leftPoint(point1), _rightPoint(point2) {
    if (_leftPoint.x() > _rightPoint.x()) {
        std::swap(_leftPoint, _rightPoint);
    }
}

ParabolaIntersectionSolutionSet ParabolaIntersectionSolutionSet::noSolution() {
    return ParabolaIntersectionSolutionSet();
}

ParabolaIntersectionSolutionSet ParabolaIntersectionSolutionSet::infiniteSolutions() {
    ParabolaIntersectionSolutionSet solutionSet;
    solutionSet._type = INFINITE_SOLUTIONS;
    return solutionSet;
}

bool ParabolaIntersectionSolutionSet::isEmpty() const {
    return _type == NO_SOLUTION;
}

bool ParabolaIntersectionSolutionSet::isOne() const {
    return _type == ONE_SOLUTION;
}

bool ParabolaIntersectionSolutionSet::isTwo() const {
    return _type == TWO_SOLUTIONS;
}

bool ParabolaIntersectionSolutionSet::isInfinite() const {
    return _type == INFINITE_SOLUTIONS;
}

const ParabolaIntersectionSolutionSet::Type &ParabolaIntersectionSolutionSet::type() const {
    return _type;
}

const Point &ParabolaIntersectionSolutionSet::point() const {
    return _leftPoint;
}

const Point &ParabolaIntersectionSolutionSet::leftPoint() const {
    return _leftPoint;
}

const Point &ParabolaIntersectionSolutionSet::rightPoint() const {
    return _rightPoint;
}

