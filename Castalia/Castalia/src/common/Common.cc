#include <Common.h>

PolygonHole::PolygonHole() :
        _id(-1) {
}

PolygonHole::PolygonHole(int id) :
        _id(id) {
}

void PolygonHole::addNode(int &n) {
    _nodeList.push_back(n);
}

int PolygonHole::id() const {
    return _id;
}

std::vector<int> PolygonHole:: nodeList() {
    return _nodeList;
}

