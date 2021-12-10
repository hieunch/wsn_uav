
#include "GlobalLocationService.h"

cModule *GlobalLocationService::networkModule;
bool GlobalLocationService::initialized = false;
double GlobalLocationService::cellWidth = 1;
double GlobalLocationService::cellHeight = 1;
std::vector<int> GlobalLocationService::numReceiveds;
std::vector<std::map<int, int>> GlobalLocationService::colors;
std::vector<Point> GlobalLocationService::locations;
std::vector<std::vector<NeighborRecord>> GlobalLocationService::neighborTables;
map<tuple<int, int>, int> GlobalLocationService::spCache;
const double range = 100;

void GlobalLocationService::initialize(cModule *module) {
  if (!initialized) {
    networkModule = module;
    initialized = true;

    int numNodes = (int) module->par("numNodes").longValue();
    locations.assign(numNodes, Point());
    neighborTables.assign(numNodes, std::vector<NeighborRecord>());
    numReceiveds.assign(numNodes, 0);
    colors.assign(numNodes, std::map<int, int>());
    for (int i = 0; i < numNodes; i++) {
      double xCoor = networkModule->getSubmodule("node", i)->par("xCoor").doubleValue();
      double yCoor = networkModule->getSubmodule("node", i)->par("yCoor").doubleValue();
      locations[i] = Point(xCoor, yCoor);
    }
    cellWidth = module->par("cellWidth").doubleValue();
    cellHeight = module->par("cellHeight").doubleValue();

    for (int i = 0; i < numNodes; i++) {
      for (int j = i + 1; j < numNodes; j++) {
        if (G::distance(locations[i], locations[j]) < range) {
          neighborTables[i].push_back(NeighborRecord(j, locations[j]));
          neighborTables[j].push_back(NeighborRecord(i, locations[i]));
        }
      }
    }
  }
}
std::vector<NeighborRecord> GlobalLocationService::getNeighborTable(int id) {
  return neighborTables[id];
}

Point GlobalLocationService::getLocation(int id) {
  if (id == -1) return Point();
  return locations[id];
}

int GlobalLocationService::getId(Point p) {
  for (int v = 0; v < locations.size(); v++) {
    if (locations[v] == p) return v;
  }
  return -1;
}

int GlobalLocationService::getNumReceived(int id) {
  if (id == -1) return 0;
  return numReceiveds[id];
}

void GlobalLocationService::increaseNumReceived(int id){
  numReceiveds[id]++;
}

int GlobalLocationService::numHopShortestPath(int source, int destination) {
  if (spCache.find(make_tuple(source, destination)) != spCache.end()) {
    return spCache[make_tuple(source, destination)];
  }

  vector<bool> marked;
  marked.assign(locations.size(), false);
  vector<int> d;
  d.assign(locations.size(), 0);

  marked[source] = true;
  queue<int> q;
  q.push(source);
  while (!q.empty()) {
    int u = q.front(); q.pop();
    if (u == destination) break;
    for (int v = 0; v < locations.size(); v++) if (!marked[v]) {
      if (G::distance(locations[u], locations[v]) < range) {
        marked[v] = true;
        d[v] = d[u] + 1;
        q.push(v);
      }
    }
  }


  spCache[make_tuple(source, destination)] = d[destination];


  return d[destination];
}

bool GlobalLocationService::isInSameCell(Point A, Point B) {
  // return ((int(A.x_/cellWidth) == int(B.x_/cellWidth)) &&
  //         (int(A.y_/cellHeight) == int(B.y_/cellHeight)));
  return G::distance(A, B) < 40;
}

vector<Point> GlobalLocationService::getCell(Point p) {
  vector<Point> cell;
  double x1 = floor(p.x_/cellWidth) * cellWidth;
  double y1 = floor(p.y_/cellHeight) * cellHeight;
  double x2 = x1 + cellWidth;
  double y2 = y1 + cellHeight;
  Point A(x1, y1); cell.push_back(A);
  Point B(x2, y1); cell.push_back(B);
  Point C(x2, y2); cell.push_back(C);
  Point D(x1, y2); cell.push_back(D);
  return cell;
}

int GlobalLocationService::getColor(int current, int destination) {
  if (colors[current].find(destination) == colors[current].end()) {
    colors[current][destination] = 0;
  }
  return colors[current][destination];
}

void GlobalLocationService::increaseColor(int current, int destination) {
  if (colors[current].find(destination) != colors[current].end()) {
    colors[current][destination]++;
  }
}

int GlobalLocationService::getColor(int node) {
  int maxColor = 0;
  std::map<int, int>::iterator it;
  for(it = colors[node].begin(); it != colors[node].end(); it++) {
    int color = it->second;
    if (color > maxColor) maxColor = color;
  }
  return maxColor;
}

double GlobalLocationService::calculatePathLength(vector<int> path) {
	double pathLength = 0;
	for (int i=0; i<path.size()-1; i++){
		pathLength += G::distance(getLocation(path[i]), getLocation(path[i+1]));
	}
	return pathLength;
}

double GlobalLocationService::convexHullPerimeter(vector<int> polygon) {
  double per = 0;
  if (polygon.size() > 1) {
    if (polygon.size() == 2) {
      per = 2*G::distance(getLocation(polygon[0]), getLocation(polygon[1]));
    } else {
      vector<Point> polygonPoint;
      for (int i : polygon) polygonPoint.push_back(getLocation(i));
      per = G::polygonPerimeter(G::convexHull(polygonPoint));
    }
  }
	return per;
}