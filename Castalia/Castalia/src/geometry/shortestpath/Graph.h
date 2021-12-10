#ifndef GEOMETRY_SHORTESTPATH_GRAPH_H_
#define GEOMETRY_SHORTESTPATH_GRAPH_H_

#include <list>
#include <queue>
#include <stack>

using namespace std;

namespace wsn {

class Graph {
private:
    int numVertices;    // number of graph's vertices
    list<int> *adj;     // adjacency list
public:
    Graph(int v);
    void addEdge(int v, int w);
    vector<int> BFS(int s, int t);
};

} /* namespace wsn */

#endif /* GEOMETRY_SHORTESTPATH_GRAPH_H_ */
