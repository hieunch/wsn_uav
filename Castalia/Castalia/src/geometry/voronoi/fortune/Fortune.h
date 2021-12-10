/*
 * Fortune.h
 *
 *  Created on: Jan 7, 2017
 *      Author: starlight
 */

#ifndef GEOMETRY_VORONOI_FORTUNE_FORTUNE_H_
#define GEOMETRY_VORONOI_FORTUNE_FORTUNE_H_

#include <vector>
#include <queue>

#include <voronoi/VoronoiDiagram.h>
#include <voronoi/fortune/Event.h>
#include <voronoi/fortune/BeachLine.h>

namespace voronoi {
    namespace fortune {

        class Fortune {
        public:
            Fortune();

            void operator()(VoronoiDiagram &diagram);

        private:
            VoronoiDiagram *diagram;

            std::priority_queue<Event *, std::vector<Event *>, EventComparator> eventQueue;
            BeachLine beachLine;
            double sweepLineY;

            void calculate();

            void addEventsFor(const std::vector<VoronoiSite *> &sites);

            void addEvent(Event *event);

            Event *nextEvent();

            void processEvent(Event *event);

            void handleSiteEvent(SiteEvent *event);

            void handleCircleEvent(CircleEvent *event);

            void checkForCircleEvent(Arc *arc);
        };

    } //end namespace fortune
} //end namespace voronoi



#endif /* GEOMETRY_VORONOI_FORTUNE_FORTUNE_H_ */
