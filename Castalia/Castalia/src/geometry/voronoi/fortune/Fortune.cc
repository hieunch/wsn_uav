/*
 * Fortune.cc
 *
 *  Created on: Jan 7, 2017
 *      Author: starlight
 */

#include <voronoi/fortune/Fortune.h>

using namespace voronoi;
using namespace voronoi::fortune;
using namespace geometry;

Fortune::Fortune()
        : diagram(0) {
}

void Fortune::operator()(VoronoiDiagram &diagram) {
    this->diagram = &diagram;
    calculate();
    this->diagram = 0;
}

void Fortune::calculate() {
    addEventsFor(diagram->sites());

    while (Event *event = nextEvent()) {
        processEvent(event);
        delete event;
    }
}

void Fortune::processEvent(Event *event) {
    sweepLineY = event->position().y();

    if (event->isSiteEvent()) {
        handleSiteEvent(event->asSiteEvent());
    } else {
        handleCircleEvent(event->asCircleEvent());
    }
}

void Fortune::addEventsFor(const std::vector<VoronoiSite *> &sites) {
    for (std::vector<VoronoiSite *>::const_iterator it = sites.begin(); it != sites.end(); ++it) {
        addEvent(new SiteEvent(*it));
    }
}

void Fortune::addEvent(Event *event) {
    eventQueue.push(event);
}

Event *Fortune::nextEvent() {
    if (eventQueue.empty()) {
        return 0;
    }

    Event *event = eventQueue.top();
    eventQueue.pop();

    return event;
}

void Fortune::handleSiteEvent(SiteEvent *event) {
    VoronoiSite *site = event->site();

    Arc *newArc = beachLine.createArc(site);

    if (beachLine.isEmpty()) {
        beachLine.insert(newArc);
        return;
    }

    Arc *arc = beachLine.arcAbove(site->position());

    if (arc) {
        arc->invalidateCircleEvent();
        newArc->setLeftEdge(diagram->createEdge(arc->site(), site));

        beachLine.splitArcWith(arc, newArc);
        //arc->splitWith(newArc);
    } else {
        Arc *last = beachLine.lastElement();
        newArc->setLeftEdge(diagram->createEdge(last->site(), site));
        //last->insert(newArc);
        beachLine.insertAfter(newArc, last);
    }

    checkForCircleEvent(newArc->prev());
    checkForCircleEvent(newArc->next());
}

void Fortune::handleCircleEvent(CircleEvent *event) {
    if (!event->isValid()) return;

    Arc *arc = event->arc();
    Arc *prev = arc->prev();
    Arc *next = arc->next();
    VoronoiEdge *leftEdge = arc->leftEdge();
    VoronoiEdge *rightEdge = arc->rightEdge();
    Point centerPoint = event->circle().center();

    VoronoiEdge *newEdge = diagram->createEdge(prev->site(), next->site());

    newEdge->adjustOrientation(arc->site()->position());
    newEdge->addPoint(centerPoint);

    leftEdge->adjustOrientation(next->site()->position());
    leftEdge->addPoint(centerPoint);

    rightEdge->adjustOrientation(prev->site()->position());
    rightEdge->addPoint(centerPoint);

    // connect half edges
    VoronoiHalfEdge *he = leftEdge->halfEdgeFor(arc->site());
    he->setStartPoint(centerPoint);
    he->setPrev(rightEdge->halfEdgeFor(arc->site()));

    he = rightEdge->halfEdgeFor(next->site());
    he->setStartPoint(centerPoint);
    he->setPrev(newEdge->halfEdgeFor(next->site()));

    he = newEdge->halfEdgeFor(prev->site());
    he->setStartPoint(centerPoint);
    he->setPrev(leftEdge->halfEdgeFor(prev->site()));

    diagram->addVertex(newEdge->edge().startPoint());
    if (newEdge->edge().isSegment())
        diagram->addVertex(newEdge->edge().endPoint());
    beachLine.replaceArc(arc, newEdge);

    checkForCircleEvent(prev);
    checkForCircleEvent(next);
}

void Fortune::checkForCircleEvent(Arc *arc) {
    if (!arc) return;
    arc->invalidateCircleEvent();

    if (!arc->hasTwoDifferentNeighborSites()) return;

    Point a = arc->prev()->site()->position();
    Point b = arc->next()->site()->position();
    Point c = arc->site()->position();

    if (pointsOnLine(a, b, c)) {
        if (a.x() >= b.x() && b.x() >= c.x()) {

        } else {
            return;
        }
    } else {
        if (clockwise(a, b, c)) return;
    }

    Circle circle(a, b, c);

    if (circle.top().y() > sweepLineY) {
        return;
    }

    addEvent(new CircleEvent(arc, circle));
}

