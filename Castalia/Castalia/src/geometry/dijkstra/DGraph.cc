#include <queue>
#include "DGraph.h"

DGraph::DGraph(vector<Point> polygon) {
    adj = new list<pair_>[polygon.size() + 2];

    for (unsigned int i = 0; i < polygon.size(); i++) {
        this->polygon.push_back(polygon[i]);
        for (unsigned int j = i + 1; j < polygon.size(); j++) {
            // if (G::isSegmentInsidePolygon(polygon[i], polygon[j], polygon) || j == i + 1) {
            if (G::inOrOnPolygon(polygon, LineSegment(polygon[i], polygon[j])) || j == i+1) {
                adj[i].push_back(make_pair(G::distance(polygon[i], polygon[j]), j));
                adj[j].push_back(make_pair(G::distance(polygon[i], polygon[j]), i));
            }
        }
    }
}

void DGraph::addVertexToPath(vector<int> parent, int j) {
    if (parent[j] != -1) {
        addVertexToPath(parent, parent[j]);
        STP.push_back(parent[j]);
    }
}

bool DGraph::isVertexOfPolygon(Point s, vector<Point> p) {
    for (auto it : p) {
        if (it.x() == s.x() && it.y() == s.y())
            return true;
    }
    return false;
}

int DGraph::getIndexInPolygon(Point s, vector<Point> p) {
    for (unsigned int i = 0; i < p.size(); i++) {
        if (p[i].x() == s.x() && p[i].y() == s.y())
            return i;
    }
    return NaN;
}
vector<Point> DGraph::shortestPath(Point s, Point t) {
    vector<Point> new_polygon;
    for (auto it : polygon) new_polygon.push_back(it);
    if (!isVertexOfPolygon(s, polygon))
        new_polygon.push_back(s);
    if (!isVertexOfPolygon(t, polygon))
        new_polygon.push_back(t);
    unsigned int s_ = getIndexInPolygon(s, new_polygon);
    unsigned int t_ = getIndexInPolygon(t, new_polygon);
    for (unsigned int i = 0; i < new_polygon.size(), i != s_; i++) {
        // if (G::isSegmentInsidePolygon(new_polygon[i], s, polygon)) {
        if (G::inOrOnPolygon(polygon, LineSegment(polygon[i], s))) {
            adj[i].push_back(make_pair(G::distance(new_polygon[i], s), s_));
            adj[s_].push_back(make_pair(G::distance(new_polygon[i], s), i));
        }
    }
    for (unsigned int j = 0; j < new_polygon.size(), j != t_; j++) {
        // if (G::isSegmentInsidePolygon(new_polygon[j], t, polygon)) {
        if (G::inOrOnPolygon(polygon, LineSegment(polygon[j], t))) {
            adj[j].push_back(make_pair(G::distance(new_polygon[j], t), t_));
            adj[t_].push_back(make_pair(G::distance(new_polygon[j], t), j));
        }
    }
    priority_queue<pair_, vector<pair_>, greater<pair_>> pq;
    vector<double> dist(new_polygon.size(), DBL_MAX);
    vector<int> parent(new_polygon.size(), -1);

    pq.push(make_pair(0, s_));
    dist[s_] = 0;

    while (!pq.empty()) {
        int u = pq.top().second;
        pq.pop();

        list<pair_>::iterator it;
        for (it = adj[u].begin(); it != adj[u].end(); it++) {
            int v = (*it).second;
            double w = (*it).first;

            if (dist[v] > dist[u] + w) {
                dist[v] = dist[u] + w;
                pq.push(make_pair(dist[v], v));
                parent[v] = u;
            }
        }
    }

    addVertexToPath(parent, t_);
    STP.push_back(t_);
    vector<Point> sp;
    for (auto it : STP)
        sp.push_back(new_polygon[it]);
    return sp;
}

vector<int> DGraph::shortestPath2(Point s, Point t) {
    vector<Point> new_polygon;
    for (auto it : polygon) new_polygon.push_back(it);
    if (!isVertexOfPolygon(s, polygon))
        new_polygon.push_back(s);
    if (!isVertexOfPolygon(t, polygon))
        new_polygon.push_back(t);
    unsigned int s_ = getIndexInPolygon(s, new_polygon);
    unsigned int t_ = getIndexInPolygon(t, new_polygon);
    for (unsigned int i = 0; i < new_polygon.size(), i != s_; i++) {
        // if (G::isSegmentInsidePolygon(new_polygon[i], s, polygon)) {
        if (G::inOrOnPolygon(polygon, LineSegment(polygon[i], s))) {
            adj[i].push_back(make_pair(G::distance(new_polygon[i], s), s_));
            adj[s_].push_back(make_pair(G::distance(new_polygon[i], s), i));
        }
    }
    for (unsigned int j = 0; j < new_polygon.size(), j != t_; j++) {
        // if (G::isSegmentInsidePolygon(new_polygon[j], t, polygon)) {
        if (G::inOrOnPolygon(polygon, LineSegment(polygon[j], t))) {
            adj[j].push_back(make_pair(G::distance(new_polygon[j], t), t_));
            adj[t_].push_back(make_pair(G::distance(new_polygon[j], t), j));
        }
    }
    priority_queue<pair_, vector<pair_>, greater<pair_>> pq;
    vector<double> dist(new_polygon.size(), DBL_MAX);
    vector<int> parent(new_polygon.size(), -1);

    pq.push(make_pair(0, s_));
    dist[s_] = 0;

    while (!pq.empty()) {
        int u = pq.top().second;
        pq.pop();

        list<pair_>::iterator it;
        for (it = adj[u].begin(); it != adj[u].end(); it++) {
            int v = (*it).second;
            double w = (*it).first;

            if (dist[v] > dist[u] + w) {
                dist[v] = dist[u] + w;
                pq.push(make_pair(dist[v], v));
                parent[v] = u;
            }
        }
    }

    addVertexToPath(parent, t_);
    STP.push_back(t_);
    return STP;
}
