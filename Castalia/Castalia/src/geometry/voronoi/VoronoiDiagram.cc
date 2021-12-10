/*
 * VoronoiDiagram.cc
 *
 *  Created on: Jan 7, 2017
 *      Author: starlight
 */

#include <Fortune.h>
#include <VoronoiDiagram.h>
#include <algorithm>
#include <iterator>
#include <utility>

using namespace voronoi;
using namespace geometry;

VoronoiDiagram::VoronoiDiagram() {
}

VoronoiDiagram::~VoronoiDiagram() {
    for (std::vector<VoronoiEdge *>::iterator it = _edges.begin(); it != _edges.end(); ++it) {
        delete *it;
    }

    for (std::map<VoronoiSite *, VoronoiCell *>::iterator it = _cells.begin(); it != _cells.end(); ++it) {
        delete it->second;
    }
}

VoronoiDiagram *VoronoiDiagram::create(std::vector<VoronoiSite *> &sites) {
    VoronoiDiagram *diagram = new VoronoiDiagram();
    diagram->initialize(sites);
    diagram->calculate();
    return diagram;
}

std::vector<VoronoiSite *> &VoronoiDiagram::sites() {
    return _sites;
}

std::vector<VoronoiEdge *> &VoronoiDiagram::edges() {
    return _edges;
}

std::map<VoronoiSite *, VoronoiCell *> &VoronoiDiagram::cells() {
    return _cells;
}

std::set<geometry::Point> &VoronoiDiagram::vertices() {
    return _vextices;
}

void VoronoiDiagram::initialize(std::vector<VoronoiSite *> &sites) {
    VoronoiDiagram::removeDuplicates(sites);
    _sites = sites;
    for (std::vector<VoronoiSite *>::iterator it = _sites.begin(); it != _sites.end(); ++it) {
        _cells[*it] = new VoronoiCell(*it);
    }
}

void VoronoiDiagram::calculate() {
    fortune::Fortune fortune;
    fortune(*this);
}

VoronoiEdge *VoronoiDiagram::createEdge(VoronoiSite *left, VoronoiSite *right) {
    VoronoiEdge *edge = new VoronoiEdge(left, right);

    _edges.push_back(edge);
    _cells[left]->edges.push_back(edge);
    _cells[right]->edges.push_back(edge);

    return edge;
}

void VoronoiDiagram::addVertex(geometry::Point p) {
    _vextices.insert(p);
}

bool compareSites(VoronoiSite *site1, VoronoiSite *site2) {
    return site1->position().x() < site2->position().x() && site1->position().y() < site2->position().y();
}

bool equalSites(VoronoiSite *site1, VoronoiSite *site2) {
    return site1->position() == site2->position();
}

void VoronoiDiagram::removeDuplicates(std::vector<VoronoiSite *> &sites) {
    std::sort(sites.begin(), sites.end(), compareSites);
    std::vector<VoronoiSite *>::iterator newEnd = std::unique(sites.begin(), sites.end(), equalSites);
    sites.resize(newEnd - sites.begin());
}


