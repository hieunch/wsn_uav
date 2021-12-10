/*
 * Arc.h
 *
 *  Created on: Jan 7, 2017
 *      Author: starlight
 */

#ifndef GEOMETRY_VORONOI_FORTUNE_ARC_H_
#define GEOMETRY_VORONOI_FORTUNE_ARC_H_

#include <voronoi/VoronoiSite.h>
#include <voronoi/VoronoiEdge.h>
#include <geometry/Parabola.h>

namespace voronoi {
    namespace fortune {

        class CircleEvent;

        class BeachLine;

        class Arc {
        public:
            friend class BeachLine;

            VoronoiSite *site() const;

            Arc *prev() const;

            Arc *next() const;

            VoronoiEdge *leftEdge() const;

            VoronoiEdge *rightEdge() const;

            geometry::Parabola parabola(double baselineY) const;

            void setLeftEdge(VoronoiEdge *leftEdge); //TODO: avoid public call

            void invalidateCircleEvent();

            void resetCircleEvent(CircleEvent *event);

            bool hasTwoDifferentNeighborSites() const;

        protected:
            BeachLine *beachLine;
            VoronoiSite *_site;
            CircleEvent *_event;
            VoronoiEdge *_leftEdge;
            Arc *_prev;
            Arc *_next;

            Arc(BeachLine *beachLine, VoronoiSite *site);
        };

    } //end namespace fortune
} //end namespace voronoi


#endif /* GEOMETRY_VORONOI_FORTUNE_ARC_H_ */
