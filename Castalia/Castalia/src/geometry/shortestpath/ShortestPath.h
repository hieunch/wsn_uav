#ifndef GEOMETRY_SHORTESTPATH_SHORTESTPATH_H_
#define GEOMETRY_SHORTESTPATH_SHORTESTPATH_H_

#include <GeoMathHelper.h>
#include <Graph.h>

using namespace std;

namespace wsn {
struct Edge {
    Point e1;
    Point e2;
    Edge(Point a, Point b) {
        e1 = a;
        e2 = b;
    }
    bool operator==(Edge e) {
        return ((e1 == e.e1 && e2 == e.e2) || (e1 == e.e2 && e2 == e.e1));
    }
};

struct Triangle: triangle {
    int id;
    Triangle() {}
    Triangle(Point a, Point b, Point c, int id_) {
        vertices[0] = a;
        vertices[1] = b;
        vertices[2] = c;
        id = id_;
    }
};

// funnel algorithm
// refers this thesis https://skatgame.net/mburo/ps/thesis_demyen_2006.pdf
// & https://graphics.stanford.edu/courses/cs268-09-winter/notes/handout7.pdf

class ShortestPath {
private:
    vector<Point> polygon;
    vector<Triangle> triangleList;
    vector<Edge> diagonalList;
    vector<Triangle> sleeve;
    Point apex;

    bool isConvex(Point, Point, Point);
    bool isEar(Point, Point, Point);
    bool isEdgeOf(Point, Point, Triangle);
    Triangle getTriangle(int id);
    bool isInteriorEdge(Edge);
    int getIndex(vector<Point>, Point);
    void calculateTriangulation();
    void findSleeve(Point, Point);
    void addVertexToFunnel(vector<Point> &, vector<Point> &, Point);
public:
    ShortestPath(vector<Point>);
    vector<Point> findShortestPath(Point, Point);
};

} /* namespace wsn */

#endif /* GEOMETRY_SHORTESTPATH_SHORTESTPATH_H_ */
