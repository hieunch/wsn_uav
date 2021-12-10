/*
 * VoronoiSite.cc
 *
 *  Created on: Jan 7, 2017
 *      Author: starlight
 */

#include <voronoi/VoronoiSite.h>

using namespace voronoi;
using namespace geometry;

VoronoiSite::VoronoiSite() {
}

VoronoiSite::VoronoiSite(const Point &position) : _position(position) {
}

VoronoiSite::VoronoiSite(double x, double y) : _position(x, y) {
}

VoronoiSite::VoronoiSite(int id, double x, double y) : _position(x, y) {
    _id = id;
}

const geometry::Point &VoronoiSite::position() const {
    return _position;
}



