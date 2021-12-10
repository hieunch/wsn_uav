/*
 * BeachLine.h
 *
 *  Created on: Jan 7, 2017
 *      Author: starlight
 */

#ifndef GEOMETRY_VORONOI_FORTUNE_BEACHLINE_H_
#define GEOMETRY_VORONOI_FORTUNE_BEACHLINE_H_

#include <voronoi/fortune/Arc.h>
#include <voronoi/VoronoiEdge.h>

namespace voronoi {
    namespace fortune {

        class BeachLine {
        public:
            BeachLine();

            Arc *createArc(VoronoiSite *site);

            bool isEmpty() const;

            void insert(Arc *arc);

            void insertAfter(Arc *newArc, Arc *after);

            void splitArcWith(Arc *arc, Arc *newArc);

            Arc *arcAbove(const geometry::Point &point) const;

            void replaceArc(Arc *arc, VoronoiEdge *edge);

            Arc *lastElement() const;

        private:
            Arc *_firstElement;
            Arc *_lastElement;

            void linkArcs(Arc *arc1, Arc *arc2);

            void unlinkArc(Arc *arc);
        };

    } //end namespace fortune
} //end namespace voronoi



#endif /* GEOMETRY_VORONOI_FORTUNE_BEACHLINE_H_ */
