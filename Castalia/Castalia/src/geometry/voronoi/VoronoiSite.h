/*
 * VoronoiSite.h
 *
 *  Created on: Jan 7, 2017
 *      Author: starlight
 */

#ifndef GEOMETRY_VORONOI_VORONOISITE_H_
#define GEOMETRY_VORONOI_VORONOISITE_H_


#include <Point.h>

namespace voronoi {

    class VoronoiSite {
    public:
        VoronoiSite();

        VoronoiSite(const geometry::Point &position);

        VoronoiSite(double x, double y);

        VoronoiSite(int id, double x, double y);

        const geometry::Point &position() const;

        const int &id() const { return _id; };

    protected:
        geometry::Point _position;
        int _id;
    };

} //end namespace voronoi




#endif /* GEOMETRY_VORONOI_VORONOISITE_H_ */
