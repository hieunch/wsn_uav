#include <ShortestPath.h>

// hole's nodes are clockwise order, in cave region, it turns to counter clockwise order
// so in cave region, hole's nodes are counter clockwise order
// convex hull's vertices are clockwise order, too

namespace wsn {

    ShortestPath::ShortestPath(vector<Point> p) {
        for (auto it : p)
            polygon.push_back(it);
    }

// vertices must be pushed in counter-clockwise order
// http://www.math-only-math.com/area-of-the-triangle-formed-by-three-co-ordinate-points.html
    bool ShortestPath::isConvex(Point a, Point b, Point c) {
        double area = a.x() * (c.y() - b.y()) + b.x() * (a.y() - c.y()) + c.x() * (b.y() - a.y());
        return area < 0;
    }

    bool ShortestPath::isEar(Point a, Point b, Point c) {
        if (!isConvex(a, b, c))     // if b isn't convex vertex, return false
            return false;
        for (unsigned int i = 0; i < polygon.size(); i++) {
            if (polygon[i] == a || polygon[i] == b || polygon[i] == c)
                continue;
            if (G::isPointLiesInTriangle(polygon[i], a, b, c))
                return false;
        }
        return true;
    }

    bool ShortestPath::isEdgeOf(Point s, Point t, Triangle tr) {
        if (s != tr.vertices[0] && s != tr.vertices[1] && s != tr.vertices[2])
            return false;
        else if (t != tr.vertices[0] && t != tr.vertices[1] && t != tr.vertices[2])
            return false;
        return true;
    }

    void ShortestPath::calculateTriangulation() {
        vector<Point> polygon_ = polygon;

        int triangleCount = 0;
        while (1) {
            if (polygon_.size() < 3) {
                break;
            }

            for (unsigned int i = 1; i < polygon_.size(); i++) {
                Point prev = polygon_[(i - 1) % polygon_.size()];
                Point curr = polygon_[i % polygon_.size()];
                Point next = polygon_[(i + 1) % polygon_.size()];

                if (isEar(prev, curr, next)) {
                    triangleList.push_back(Triangle(curr, prev, next, ++triangleCount));
                    polygon_.erase(polygon_.begin() + i); // erase current vertex from polygon_
                }
            }
        }
    }
    void ShortestPath::findSleeve(Point s, Point t) {
        int start, end; // id of triangle contains s, t respectively
        Graph *dualGraph = new Graph(triangleList.size() + 1);

//1. construct dual graph with node is id of triangle && find triangles contain s and t
        for (unsigned int i = 0; i < triangleList.size(); i++) {
            for (unsigned int j = 0; j < triangleList.size(); j++) {    // 2 triangle has the common edge become 2 adjacent nodes of dual graph
                if (isEdgeOf(triangleList[i].vertices[1],triangleList[i].vertices[2], triangleList[j])
                    || isEdgeOf(triangleList[i].vertices[1], triangleList[i].vertices[0], triangleList[j])
                    || isEdgeOf(triangleList[i].vertices[0], triangleList[i].vertices[2], triangleList[j])) {
                    dualGraph->addEdge(triangleList[i].id, triangleList[j].id);
                }
            }
            if (G::isPointLiesInTriangle(s, triangleList[i].vertices[0], triangleList[i].vertices[1], triangleList[i].vertices[2])) {
                start = triangleList[i].id;
            }
            if (G::isPointLiesInTriangle(t, triangleList[i].vertices[0], triangleList[i].vertices[1], triangleList[i].vertices[2])) {
                end = triangleList[i].id;
            }
        }

//2. find sleeve from s to t
        vector<int> sleeve_ = dualGraph->BFS(start, end);
        for (auto it : sleeve_) {
            Triangle tr_ = getTriangle(it);
            sleeve.push_back(tr_);
        }

//3. construct diagonal list
        // take the common edges of 2 consecutive triangle
        for (unsigned int i = 0; i < sleeve.size() - 1; i++) {
            if (isEdgeOf(sleeve[i].vertices[0], sleeve[i].vertices[1], sleeve[i + 1])) {
                diagonalList.push_back(Edge(sleeve[i].vertices[0], sleeve[i].vertices[1]));
            }
            if (isEdgeOf(sleeve[i].vertices[1], sleeve[i].vertices[2], sleeve[i + 1])) {
                diagonalList.push_back(Edge(sleeve[i].vertices[1], sleeve[i].vertices[2]));
            }
            if (isEdgeOf(sleeve[i].vertices[2], sleeve[i].vertices[0], sleeve[i + 1])) {
                diagonalList.push_back(Edge(sleeve[i].vertices[2], sleeve[i].vertices[0]));
            }
        }
    }

    bool ShortestPath::isInteriorEdge(Edge e) {
        for (unsigned int i = 0; i < diagonalList.size(); i++)
            if (diagonalList[i] == e)
                return true;
        return false;
    }

// get triangle by it's id
    Triangle ShortestPath::getTriangle(int id) {
        for (unsigned int i = 0; i < triangleList.size(); i++) {
            if (triangleList[i].id == id)
                return triangleList[i];
        }
        return Triangle();
    }

    int ShortestPath::getIndex(vector<Point> p, Point t) {
        for (unsigned int i = 0; i < p.size(); i++)
            if (p[i] == t)
                return i;
        return NaN;
    }

    vector<Point> ShortestPath::findShortestPath(Point s, Point t) {
        calculateTriangulation();
        findSleeve(s, t);
        apex = s;

        vector<Point> leftChain, rightChain;
        vector<Point> sp1; //shortest path from s to left chain
        vector<Point> sp2; //shortest path form s to right chain

        if (sleeve.size() <= 1) {
            sp1.push_back(s);
            sp1.push_back(t);
            return sp1;
        }

        // add last diagonal - diagonal that contains t
        diagonalList.push_back(Edge(diagonalList[diagonalList.size() - 1].e2,t));

        // F(0)
        sp1.push_back(s);
        sp1.push_back(diagonalList[0].e1);
        leftChain.push_back(diagonalList[0].e1);
        sp2.push_back(s);
        sp2.push_back(diagonalList[0].e2);
        rightChain.push_back(diagonalList[0].e2);

        // F(i)
        for (unsigned int i = 1; i < diagonalList.size(); i++) {
            Point e1_ = diagonalList[i].e1;
            Point e2_ = diagonalList[i].e2;
            Point left, right;
            if (!isInteriorEdge(Edge(leftChain[leftChain.size() - 1], e1_))) {
                left = e1_;
                right = e2_;
            } else {
                left = e2_;
                right = e1_;
            }
            if (left == leftChain[leftChain.size() - 1]) {
                rightChain.push_back(right);
                addVertexToFunnel(sp2, sp1, right);
            } else {
                leftChain.push_back(left);
                addVertexToFunnel(sp1, sp2, left);
            }
        }

        if (sp1[sp1.size() - 1] == t) return sp1;
        else if (sp2[sp2.size() - 1] == t) return sp2;
        else return vector<Point>();
    }

    void ShortestPath::addVertexToFunnel(vector<Point> &p1, vector<Point> &p2, Point p) {
        bool changeApex = false;
        int a = getIndex(p1, apex);
        for (unsigned int j = a; j < p1.size(); j++) {
            if (G::isSegmentInsidePolygon(p1[j], p, polygon)) {
                p1.erase(p1.begin() + j + 1, p1.end());
                p1.push_back(p);
                break;
            }
        }

        if (p1.size() > 2) {
            Point m = p1[p1.size() - 2];
            unsigned int k = getIndex(polygon, m);
            unsigned int k1 = (k == 0) ? (polygon.size() - 1) : (k - 1);
            unsigned int k2 = (k == polygon.size() - 1) ? 0 : (k + 1);

            if (G::angle(polygon[k], polygon[k1], polygon[k2]) <= M_PI) {
                if (G::is_intersect(polygon[k1], polygon[k2], polygon[k], p1[p1.size() - 1])) {
                        changeApex = true;
                }
                else if (G::isPointLiesInTriangle(p1[p1.size() - 1], polygon[k1], polygon[k2], polygon[k]))
                    changeApex = true;
            } else {
                if (!G::is_intersect(polygon[k1], polygon[k2], polygon[k], p1[p1.size() - 1]) &&
                    !G::isPointLiesInTriangle(p1[p1.size() - 1], polygon[k1], polygon[k2], polygon[k]))
                    changeApex = true;
            }
            if (changeApex) {
                a = getIndex(p2, apex);
                for (unsigned int j = a; j < p2.size(); j++) {
                    if (G::isSegmentInsidePolygon(p2[j], p, polygon)) {
                        vector<Point>().swap(p1);
                        for (unsigned int h = 0; h <= j; h++)
                            p1.push_back(p2[h]);
                        p1.push_back(p);
                        apex = p2[j];
                        break;
                    }
                }
            }
        }
    }


} /* namespace wsn */
