#include<GeoMathHelper.h>
#include<GlobalLocationService.h>
#include<list>
#include<vector>
#include<map>
#include <stdio.h>

using namespace std;

class UDG {

public:
    int V;
    int E;
    double d0;
    int sinkId;
    
protected:
    map<int, double> nodes;
    map<int, list<int>> adj;
    map<int, map<int, double>> length;
    map<int, double> weight;

public:
    UDG() {}

    void init(int numNodes, int sinkId, double d0){
        V = numNodes;
        this->d0 = d0;
        this->sinkId = sinkId;

        for (int v = 0; v < numNodes; v++) {
            addNode(v);
        }
        for (int v = 0; v < numNodes; v++) {
            for (int u = v+1; u < numNodes; u++) {
                double d = G::distance(GlobalLocationService::getLocation(u), GlobalLocationService::getLocation(v));
                if (d < d0) {
                    addEdge(v, u);
                }
            }
        }
    }

    void init2(int numNodes, int sinkId, vector<int> A, vector<int> cent) {
        V = numNodes;
        this->sinkId = sinkId;

        for (int l : A) addNode(l);
        for (int l : A) {
            for (int v=0; v<numNodes; v++) {
                if (cent[v] != l) continue;
                Point p_v = GlobalLocationService::getLocation(v);
                for (int w=0; w<numNodes; w++) {
                    if ((cent[w] == l) || (cent[w] == -1)) continue;
                    Point p_w = GlobalLocationService::getLocation(w);
                    double dist = G::distance(p_v, p_w);
                    if (dist > 25) continue;
                    int l2 = cent[w];
                    if (hasEdge(l, l2)) continue;
                    addEdge(l, l2);
                }
            }
        }
    }

    void init3(int numNodes, int sinkId, vector<int> A, vector<int> cent) {
        V = numNodes;
        this->sinkId = sinkId;
        
        for (int l : A) addNode(l);
        for (int l : A) {
            for (int l2 : A) {
                if (l2 >= l) continue;
                double d = G::distance(GlobalLocationService::getLocation(l), GlobalLocationService::getLocation(l2));
                if (d < 100) {
                    addEdge(l, l2);
                }
            }
        }
    }

    void addNode(int u) { addNode(u, 0); }

    void addNode(int u, double E_u) {
        nodes.insert(pair<int, double>(u, E_u));
        adj.insert(pair<int, list<int>>(u, list<int>()));
        length.insert(pair<int, map<int, double>>(u, map<int, double>()));
        weight.insert(pair<int, double>(u, 0));
    }

    void addEdge(int v, int w) {
        validateVertex(v);
        validateVertex(w);
        E++;
        adj.find(v)->second.push_back(w);
        adj.find(w)->second.push_back(v);
        double d = G::distance(GlobalLocationService::getLocation(v), GlobalLocationService::getLocation(w));
        // for (int rr : ranges) {
        //     if (rr > d) {
        //         d = rr;
        //         break;
        //     }
        // }
        length.find(v)->second.insert(pair<int,double>(w, d));
        length.find(w)->second.insert(pair<int,double>(v, d));
    }

    bool hasEdge(int v, int w) {
        list<int> adj_v = adj.find(v)->second;
        for (int w2 : adj_v) {
            if (w2 == w) return true;
        }
        return false;
        // return (G::distance(GlobalLocationService::getLocation(v), GlobalLocationService::getLocation(w)) < 100);
    }

    list<int> getAdj(int v) { return adj.find(v)->second; }

    list<int> getAdjExceptSink(int v) {
        list<int> adj_v;
        for (int w : adj.find(v)->second) {
            if (w != sinkId) adj_v.push_back(w);
        }
        return adj_v; 
    }

    double getLength(int u, int v){
        return length.find(u)->second.find(v)->second;
    }

    void setWeight(int u, double w){
        weight.find(u)->second = w;
    }

    double getWeight(int u){
        return weight.find(u)->second;
    }

    double getTotalWeights(){
        double totalWeights = 0;
        for (int i=0; i<V-1; i++){
            totalWeights += weight.find(i)->second;
        }
        return totalWeights;
    }

    double getE(int u){
        return nodes.find(u)->second;
    }

    list<int> getNodesExceptSink(){
        list<int> nodeList;
        for (auto pair : nodes) {
            if (pair.first != sinkId) nodeList.push_back(pair.first);
        }
        return nodeList;
    }

    vector<int> getBroadcastTree() {
        return shortestPathTree(sinkId);
    }

private: 
    void validateVertex(int v) {
        // if (v < 0 || v >= V)
        //     throw ("vertex " + v + " is not between 0 and " + (V - 1);
    }

    vector<int> shortestPathTree(int u) {
        FILE *fp;
        fp = fopen("/shortestPathTree.txt", "w+");

        queue <int> queue;
        vector<int> visited (V, 0);
        vector<int> d (V, 0);
        vector<int> trace (V, -1);
        queue.push(u);
        visited[u] = 1;
        while(!queue.empty()) {
            int uNode = queue.front();
            queue.pop();

            for (int vNode : getAdj(uNode)) {
                if (!visited[vNode]) {
                    visited[vNode] = 1;
                    d[vNode] = d[uNode]+1;
                    trace[vNode] = uNode;
                    queue.push(vNode);
                }
                else if ((d[vNode] == d[uNode]+1) && 
                    (G::distance(GlobalLocationService::getLocation(vNode), GlobalLocationService::getLocation(uNode))
                    < G::distance(GlobalLocationService::getLocation(vNode), GlobalLocationService::getLocation(trace[vNode])))){
                        trace[vNode] = uNode;
                    }
            }
            fprintf(fp, "queue %d\n", queue.size());
        }
        fclose(fp);
        return trace;
    }
};
