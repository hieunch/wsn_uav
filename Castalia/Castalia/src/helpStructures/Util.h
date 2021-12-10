#ifndef UTIL_H__
#define UTIL_H__

#include <vector>
#include <string>
#include <algorithm>
#include <sstream>
#include "GeoMathHelper.h"
#include "VirtualRouting.h"
#include <iterator>
#include "GlobalLocationService.h"
using namespace std;

class Util {
public:
  static string intVectorToString(vector<int> xs) {
    stringstream res;
    copy(xs.begin(), xs.end(), ostream_iterator<int>(res, " "));

    return res.str();
  }

  static vector<int> stringToIntVector(string s) {
    std::stringstream iss(s);

    int number;
    std::vector<int> res;
    while (iss >> number)
      res.push_back(number);

    return res;
  }

//  static int findNextHopRollingBall(Point pivot, Point ballCenter, double ballRadius, vector<NeighborRecord> candidates, Point &nextCenter) {
//    int nextHop = -1;
//    double ballCenterAngle = G::norm(atan2(ballCenter.y() - pivot.y(), ballCenter.x() - pivot.x()));
//    double minAngle = 3 * M_PI;
//    for (auto &n: candidates) {
//      Point candidateCenter = nearestCenter(pivot, n.location, ballCenter, ballRadius);
//      double candidateAngle = G::norm(atan2(candidateCenter.y() - pivot.y(), candidateCenter.x() - pivot.x()));
//      if (G::norm(candidateAngle - ballCenterAngle) < minAngle) {
//        minAngle = G::norm(candidateAngle - ballCenterAngle);
//        nextHop = n.id;
//        nextCenter = candidateCenter;
//      }
//    }
//
//    return nextHop;
//  }

//  static Point nearestCenter(Point pivot, Point next, Point center, double ballRadius) {
//    Point center1, center2;
//    G::centers(pivot, next, ballRadius, center1, center2);
//
//    double pivotAngle = G::norm(atan2(pivot.y() - center1.y(), pivot.x() - center1.x()));
//    double nextAngle = G::norm(atan2(next.y() - center1.y(), next.x() - center1.x()));
//    double diffCCWAngle = G::norm(pivotAngle - nextAngle);
//
//    if (diffCCWAngle < M_PI) {
//      return center1;
//    } else {
//      return center2;
//    }
//  }
};



#endif
