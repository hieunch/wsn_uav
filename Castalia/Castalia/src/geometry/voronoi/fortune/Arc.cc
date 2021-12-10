/*
 * Arc.cc
 *
 *  Created on: Jan 7, 2017
 *      Author: starlight
 */

#include <voronoi/fortune/Arc.h>
#include <voronoi/fortune/Event.h>
#include <voronoi/fortune/BeachLine.h>

using namespace voronoi;
using namespace voronoi::fortune;
using namespace geometry;

Arc::Arc(BeachLine *beachLine, VoronoiSite *site) : beachLine(beachLine), _site(site) {
    _prev = _next = 0;
    _event = 0;
    _leftEdge = 0;
}

Arc *Arc::prev() const {
    return _prev;
}

Arc *Arc::next() const {
    return _next;
}

VoronoiSite *Arc::site() const {
    return _site;
}

VoronoiEdge *Arc::leftEdge() const {
    return _leftEdge;
}

VoronoiEdge *Arc::rightEdge() const {
    if (!_next) return 0;
    return _next->_leftEdge;
}

Parabola Arc::parabola(double baselineY) const {
    return Parabola(_site->position(), baselineY);
}

void Arc::setLeftEdge(VoronoiEdge *leftEdge) {
    _leftEdge = leftEdge;
}

bool Arc::hasTwoDifferentNeighborSites() const {
    return _prev && _next && _prev->_site != _next->_site;
}

void Arc::invalidateCircleEvent() {
    if (!_event) return;
    _event->invalidate();
    _event = 0;
}

void Arc::resetCircleEvent(CircleEvent *event) {
    invalidateCircleEvent();
    _event = event;
}



