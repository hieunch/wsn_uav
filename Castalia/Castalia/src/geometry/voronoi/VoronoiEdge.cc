/*
 * VoronoiEdge.cc
 *
 *  Created on: Jan 7, 2017
 *      Author: starlight
 */

#include <voronoi/VoronoiEdge.h>
#include <Vector.h>

using namespace voronoi;
using namespace geometry;

VoronoiEdge::VoronoiEdge(VoronoiSite *left, VoronoiSite *right) : left(left), right(right) {
    line = Line::forNormal(left->position().midPoint(right->position()), left->position() - right->position());
    halfEdge1 = new VoronoiHalfEdge(left);
    halfEdge2 = new VoronoiHalfEdge(right);
    halfEdge1->setOpposite(halfEdge2);
}

VoronoiEdge::~VoronoiEdge() {
    delete halfEdge1;
    delete halfEdge2;
}

VoronoiHalfEdge *VoronoiEdge::halfEdgeFor(VoronoiSite *site) {
    if (site == halfEdge1->site()) return halfEdge1;
    if (site == halfEdge2->site()) return halfEdge2;
    return 0;
}

void VoronoiEdge::addPoint(const Point &point) {
    line.addPoint(point);
}

void VoronoiEdge::adjustOrientation(const Point &awayPoint) {
    if (Line::segment(left->position(), right->position()).sameSide(line.startPoint() + line.direction(), awayPoint)) {
        line.invertDirection();
    }
}

VoronoiHalfEdge::VoronoiHalfEdge(VoronoiSite *site) : _site(site), _prev(0), _next(0), _opposite(0), _hasStart(false) {
}

VoronoiSite *VoronoiHalfEdge::site() const {
    return _site;
}

VoronoiHalfEdge *VoronoiHalfEdge::opposite() const {
    return _opposite;
}

VoronoiHalfEdge *VoronoiHalfEdge::next() const {
    return _next;
}

VoronoiHalfEdge *VoronoiHalfEdge::prev() const {
    return _prev;
}

const geometry::Point &VoronoiHalfEdge::startPoint() const {
    return _startPoint;
}

bool VoronoiHalfEdge::hasStartPoint() const {
    return _hasStart;
}

void VoronoiHalfEdge::setOpposite(VoronoiHalfEdge *opposite) {
    _opposite = opposite;
    _opposite->_opposite = this;
}

void VoronoiHalfEdge::setNext(VoronoiHalfEdge *next) {
    _next = next;
    _next->_prev = this;
}

void VoronoiHalfEdge::setPrev(VoronoiHalfEdge *prev) {
    _prev = prev;
    _prev->_next = this;
}

void VoronoiHalfEdge::setStartPoint(const geometry::Point &point) {
    _startPoint = point;
    if (!_hasStart) _hasStart = true;
}

VoronoiHalfEdge *VoronoiHalfEdge::begin() {
    VoronoiHalfEdge *start = this;
    VoronoiHalfEdge *he = start;
    while (he->_prev) {
        he = he->_prev;
        if (he == start) break;
    }
    if (!he->_hasStart) he = he->next();
    return he;
}


