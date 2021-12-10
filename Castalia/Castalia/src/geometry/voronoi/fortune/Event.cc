/*
 * Event.cc
 *
 *  Created on: Jan 7, 2017
 *      Author: starlight
 */

#include <voronoi/fortune/Event.h>

using namespace voronoi;
using namespace voronoi::fortune;
using namespace geometry;

Event::~Event() {
}

bool Event::isSiteEvent() const {
    return false;
}

bool Event::isCircleEvent() const {
    return false;
}

bool Event::operator<(const Event &event) const {
    Point p1 = position();
    Point p2 = event.position();

    return p1.y() == p2.y() ? p1.x() > p2.x() : p1.y() < p2.y();
}

SiteEvent *Event::asSiteEvent() const {
    return isSiteEvent() ? (SiteEvent *) this : 0;
}

CircleEvent *Event::asCircleEvent() const {
    return isCircleEvent() ? (CircleEvent *) this : 0;
}

SiteEvent::SiteEvent(VoronoiSite *site) : _site(site) {
}

bool SiteEvent::isSiteEvent() const {
    return true;
}

Point SiteEvent::position() const {
    return _site->position();
}

VoronoiSite *SiteEvent::site() const {
    return _site;
}

CircleEvent::CircleEvent(Arc *arc, Circle circle) : valid(true), _arc(arc), _circle(circle) {
    arc->resetCircleEvent(this);
}

bool CircleEvent::isCircleEvent() const {
    return true;
}

Point CircleEvent::position() const {
    return _circle.center() - Vector(0, _circle.radius());
}

bool CircleEvent::isValid() const {
    return valid;
}

void CircleEvent::invalidate() {
    valid = false;
}

Arc *CircleEvent::arc() const {
    return _arc;
}

const Circle &CircleEvent::circle() const {
    return _circle;
}

bool EventComparator::operator()(Event *event1, Event *event2) {
    return *event1 < *event2;
}


