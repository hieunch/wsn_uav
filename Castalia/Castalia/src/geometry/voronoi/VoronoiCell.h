/*
 * VoronoiCell.h
 *
 *  Created on: Jan 7, 2017
 *      Author: starlight
 */

#ifndef GEOMETRY_VORONOI_VORONOICELL_H_
#define GEOMETRY_VORONOI_VORONOICELL_H_

#include <voronoi/VoronoiEdge.h>
#include <vector>

namespace voronoi {

    class VoronoiCell {
    public:
        VoronoiCell(VoronoiSite *site);

        VoronoiSite *site;
        std::vector<VoronoiEdge *> edges;
    };

} //end namespace voronoi


#endif /* GEOMETRY_VORONOI_VORONOICELL_H_ */
