#include <shortestpath/Graph.h>

namespace wsn {

Graph::Graph(int v) {
    this->numVertices = v;
    adj = new list<int> [v];
}

void Graph::addEdge(int v, int w) {
    adj[v].push_back(w);
}

vector<int> Graph::BFS(int s, int t) {
    //traverse graph
    bool *flag = new bool[numVertices];
    int *pred = new int[numVertices];
    for (int i = 0; i < numVertices; i++) {
        flag[i] = false;
        pred[i] = -1;
    }
    queue<int> queue_;
    flag[s] = true;
    queue_.push(s);
    while (!queue_.empty()) {
        int v = queue_.front();
        queue_.pop();
        list<int>::iterator i;
        for (i = adj[v].begin(); i != adj[v].end(); i++) {
            if (flag[*i] == false) {
                flag[*i] = true;
                pred[*i] = v;
                queue_.push(*i);
            }
        }
    }

    //find path from s to t
    stack<int> path;
    int current = t;
    while (pred[current] != -1) {
        path.push(current);
        current = pred[current];
    }
    path.push(s);

    vector<int> trace;

    while (!path.empty()) {
        int k = path.top();
        trace.push_back(k);
        path.pop();
    }

    return trace;
}

} /* namespace wsn */
