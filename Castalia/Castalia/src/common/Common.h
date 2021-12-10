#ifndef COMMON_COMMON_H_
#define COMMON_COMMON_H_

#include <vector>

#define UDP_PORT    269
#define INFINITE_TTL 10e6

struct StuckAngle {
    int a_;
    int b_;

    StuckAngle(int a, int b) : a_(a), b_(b) {}
};

class PolygonHole {
private:
    int _id;
    std::vector<int> _nodeList;
public:
    PolygonHole(int id);
    PolygonHole();
    void addNode(int& node);
    int id() const;
    std::vector<int> nodeList();
};

#endif /* COMMON_COMMON_H_ */
