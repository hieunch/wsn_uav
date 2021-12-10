#include <Line.h>
#include <algorithm>

using namespace geometry;

Line::Line() :
        _type(NULL_LINE) {
}

Line::Line(Type type, const Point &start, const Point &end,
        const Vector &direction) :
        _type(type), _startPoint(start), _endPoint(end), _direction(direction) {
}

Line Line::segment(const Point &point1, const Point &point2) {
    return Line(SEGMENT, point1, point2, point2 - point1);
}

Line Line::ray(const Point &supportVector, const Vector &direction) {
    return Line(RAY, supportVector, supportVector + direction, direction);
}

Line Line::forDirection(const Point &supportVector, const Vector &direction) {
    return Line(LINE, supportVector, supportVector + direction, direction);
}

Line Line::forNormal(const Point &supportVector, const Vector &normal) {
    return Line::forDirection(supportVector, normal.perpendicular());
}

const Point &Line::supportVector() const {
    return _startPoint;
}

const Point &Line::startPoint() const {
    return _startPoint;
}

const Vector &Line::direction() const {
    return _direction;
}

const Point &Line::endPoint() const {
    return _endPoint;
}

Line::Type Line::type() const {
    return _type;
}

void Line::setStartPoint(const Point &point) {
    _startPoint = point;
    _direction = _endPoint - _startPoint;
}

void Line::setEndPoint(const Point &point) {
    _endPoint = point;
    _direction = _endPoint - _startPoint;
}

void Line::setDirection(const Vector &direction) {
    _direction = direction;
    _endPoint = _startPoint + _direction;
}

bool Line::isNull() const {
    return _type == NULL_LINE;
}

bool Line::isLine() const {
    return _type == LINE;
}

bool Line::isRay() const {
    return _type == RAY;
}

bool Line::isSegment() const {
    return _type == SEGMENT;
}

Line Line::asLine() const {
    return Line::forDirection(_startPoint, _direction);
}

void Line::invertDirection() {
    _direction *= -1;
    if (isRay()) {
        _endPoint = _startPoint + _direction;
    } else {
        std::swap(_startPoint, _endPoint);
    }
}

bool Line::addPoint(const Point &point) {
    switch (_type) {
    case LINE:
        _startPoint = point;
        _type = RAY;
        return true;
    case RAY:
        _endPoint = point;
        _direction = _endPoint - _startPoint;
        _type = SEGMENT;
        return true;
    default:
        return false;
    }
}

LineIntersectionSolutionSet Line::intersection(const Line &line) const {
    double s;
    double t;
    if (!intersectionCoefficient(line, s)
            || !line.intersectionCoefficient(*this, t)) {
        return overlaps(line) ?
                LineIntersectionSolutionSet::infiniteSolutions() :
                LineIntersectionSolutionSet::noSolution();
    }

    if (!containsCoefficient(s) || !line.containsCoefficient(t)) {
        return LineIntersectionSolutionSet::noSolution();
    }

    return LineIntersectionSolutionSet(_startPoint + s * _direction);
}

LineIntersectionSolutionSet Line::lineIntersection(const Line &line) const {
    double s;
    if (!intersectionCoefficient(line, s)) {
        return lineContains(line.supportVector()) ?
                LineIntersectionSolutionSet::infiniteSolutions() :
                LineIntersectionSolutionSet::noSolution();
    }

    return LineIntersectionSolutionSet(_startPoint + s * _direction);
}

bool Line::intersectionCoefficient(const Line &line,
        double &coefficient) const {
    Vector u = _direction;
    Vector v = line._direction;
    Vector w = _startPoint - line._startPoint;
    double denominator = v.x() * u.y() - v.y() * u.x();
    if (denominator == 0) {
        return false;
    }
    coefficient = (v.y() * w.x() - v.x() * w.y()) / denominator;
    return true;
}

bool Line::containsCoefficient(double coefficient) const {
    switch (_type) {
    case LINE:
        return true;
    case RAY:
        return coefficient >= 0;
    case SEGMENT:
        return coefficient >= 0 && coefficient <= 1;
    default:
        return false;
    }
}

Vector Line::normal() const {
    return _direction.perpendicular();
}

Line Line::perpendicular(const Point &point) const {
    return Line::forDirection(point, normal());
}

bool Line::lineContains(const Point &p) const {
    return _direction.isParallelTo(p - _startPoint);
}

bool Line::isParallelTo(const Line &line) const {
    return _direction.isParallelTo(line.direction());
}

bool Line::contains(const Point &p) const {
    if (!lineContains(p)) {
        return false;
    }

    return containsCoefficient(coefficientForPointOnLine(p));
}

bool Line::overlaps(const Line &line) const {
    if (!isParallelTo(line) || !lineContains(line.startPoint())) {
        return false;
    }

    return containsCoefficient(coefficientForPointOnLine(line.startPoint()))
            || containsCoefficient(coefficientForPointOnLine(line.endPoint()))
            || line.containsCoefficient(
                    line.coefficientForPointOnLine(_startPoint))
            || line.containsCoefficient(
                    line.coefficientForPointOnLine(_endPoint));
}

bool Line::sameSide(const Point &p1, const Point &p2) const {
    Vector v1 = p1 - lineIntersection(perpendicular(p1)).point();
    Vector v2 = p2 - lineIntersection(perpendicular(p2)).point();

    return v1.dotProduct(v2) > 0;
}

double Line::coefficientForPointOnLine(const Point &p) const {
    double directionLength = _direction.length();

    if (directionLength <= 0) {
        return 0;
    }

    return (p - _startPoint).length() / directionLength;
}

LineIntersectionSolutionSet::LineIntersectionSolutionSet() :
        _type(NO_SOLUTION) {
}

LineIntersectionSolutionSet::LineIntersectionSolutionSet(const Point &point) :
        _point(point), _type(ONE_SOLUTION) {
}

LineIntersectionSolutionSet LineIntersectionSolutionSet::noSolution() {
    return LineIntersectionSolutionSet();
}

LineIntersectionSolutionSet LineIntersectionSolutionSet::infiniteSolutions() {
    LineIntersectionSolutionSet solutionSet;
    solutionSet._type = INFINITE_SOLUTIONS;
    return solutionSet;
}

bool LineIntersectionSolutionSet::isEmpty() const {
    return _type == NO_SOLUTION;
}

bool LineIntersectionSolutionSet::isOne() const {
    return _type == ONE_SOLUTION;
}

bool LineIntersectionSolutionSet::isInfinite() const {
    return _type == INFINITE_SOLUTIONS;
}

LineIntersectionSolutionSet::Type LineIntersectionSolutionSet::type() const {
    return _type;
}

const Point &LineIntersectionSolutionSet::point() const {
    return _point;
}
