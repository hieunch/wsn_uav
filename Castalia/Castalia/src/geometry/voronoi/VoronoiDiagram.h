/*
 * VoronoiDiagram.h
 *
 *  Created on: Jan 7, 2017
 *      Author: starlight
 */

#ifndef GEOMETRY_VORONOI_VORONOIDIAGRAM_H_
#define GEOMETRY_VORONOI_VORONOIDIAGRAM_H_

#include <Point.h>
#include <voronoi/VoronoiCell.h>
#include <voronoi/VoronoiEdge.h>
#include <voronoi/VoronoiSite.h>
#include <map>
#include <set>
#include <vector>

namespace voronoi {

    class VoronoiDiagram {
    public:
        VoronoiDiagram();

        ~VoronoiDiagram();

        static VoronoiDiagram *create(std::vector<VoronoiSite *> &sites);

        std::vector<VoronoiSite *> &sites();

        std::vector<VoronoiEdge *> &edges();

        std::map<VoronoiSite *, VoronoiCell *> &cells();

        std::set<geometry::Point> &vertices();

        void initialize(std::vector<VoronoiSite *> &sites);

        void calculate();

        VoronoiEdge *createEdge(VoronoiSite *left, VoronoiSite *right);

        void addVertex(geometry::Point);

    protected:
        std::vector<VoronoiSite *> _sites;
        std::vector<VoronoiEdge *> _edges;
        std::map<VoronoiSite *, VoronoiCell *> _cells;
        std::set<geometry::Point> _vextices;

        static void removeDuplicates(std::vector<VoronoiSite *> &sites);
    };

} //end namespace voronoi

#endif /* GEOMETRY_VORONOI_VORONOIDIAGRAM_H_ */
