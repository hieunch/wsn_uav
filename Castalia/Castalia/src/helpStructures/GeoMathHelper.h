#ifndef GEOMETRY_GEOMATHHELPER_H_
#define GEOMETRY_GEOMATHHELPER_H_

#define DEBUG(x) cout << #x << " = " << x << endl;
#define DEBUGA(a, b) cout << "[ "; for (auto x = (a); x != (b); x++) cout << *x << " ";\
  cout << "]" << endl

#include <float.h>
#include <math.h>
#include <sys/types.h>
#include <set>
#include <map>
#include <limits>
#include <stack>
#include <algorithm>
#include <vector>
#include <iostream>
#include <tuple>        // std::tuple, std::make_tuple, std::tie

#define g_min(x,y) (((x)<(y))?(x):(y))
#define g_max(x,y) (((x)>(y))?(x):(y))
#define in_range(x,a,y) ((x) <= (a) && (a) <= (y)) || ((x) >= (a) && (a) >= (y))

#define EPSILON 0.0001
#define INVALID -10000
#define M_PI 3.141592653
#define M_PI_2 3.141592653/2

#ifndef NaN
#define NaN std::numeric_limits<double>::quiet_NaN()
#endif

typedef double Angle;
class G;

using namespace std;
struct Vector;
struct DirectedSegment;
struct DirectedArc;
struct Ellipse;

struct Point {
  Point() {
    x_ = INVALID;
    y_ = INVALID;
  }

  Point(double x, double y) {
    x_ = x;
    y_ = y;
  }

  inline double x() const {
    return x_;
  }

  bool isUnspecified() {
    return x_ == INVALID;
  }

  inline double y() const {
    return y_;
  }

  double x_;
  double y_;

  inline bool operator==(const Point& rhs) const {
    return x_ == rhs.x_ && y_ == rhs.y_;
  }
  inline bool operator!=(const Point& rhs) const {
    return !operator==(rhs);
  }
  inline bool operator<(const Point& rhs) const {
    return x_ < rhs.x_ || (x_ == rhs.x_ && y_ < rhs.y_);
  }
  inline bool operator>(const Point& rhs) const {
    return x_ > rhs.x_ || (x_ == rhs.x_ && y_ > rhs.y_);
  }

  friend std::ostream& operator<< ( std::ostream& os, const Point& c );

  Point operator+(Vector v);

};

struct CavernNode : Point {
    int id_;
    double level_;
};

struct Vector {
  double x, y;
  Vector(Point A, Point B) {
    this->x = B.x() - A.x();
    this->y = B.y() - A.y();
  }

  Vector() {}

  Vector(double x, double y): x(x), y(y) {}

  double length() {
    return sqrt(x * x + y * y);
  }

  Vector operator*(const double a) {
    return Vector(a * x, a * y);
  }

  Vector rotate(double alpha) {
    return Vector(this->x * cos(alpha) - this->y * sin(alpha), this->y * cos(alpha) + this->x * sin(alpha));
  }

  Point operator+(Point p) {
    return Point(x + p.x(), y + p.y());
  }

  double crossProduct(Vector other) {
    return x * other.y - y * other.x;
  }

  double dotProduct(Vector other) {
    return x * other.x + y * other.y;
  }

  friend std::ostream& operator<< ( std::ostream& os, const Vector& v );
};

struct triangle {
	Point vertices[3];
};

struct Circle {
  Point center;
  double radius;
  Circle(Point center, double radius): center(center), radius(radius) {}
};

struct Circle2: Point {
    double radius_;
};

struct LineSegment {
  Point u, v;
  LineSegment(Point u, Point v): u(u), v(v) {}
};

struct DirectedSegment {
  Point from;
  Point to;
  DirectedSegment() {};
  DirectedSegment(Point from, Point to): from(from), to(to) {}
};


struct DirectedArc {
  // at most PI, clockwise order
  Point from;
  Point to;
  Point center;
  double radius;
  DirectedArc() {};
  DirectedArc(Point from, Point to, Point center, double radius): from(from), to(to), center(center), radius(radius) {}
};


struct Ray {
  Point p;
  Vector v;
  Ray(Point p, Vector v): p(p), v(v) {}
};


struct NeighborRecord {
  int id;      // the node's ID
  Point location;
  NeighborRecord(){}
  NeighborRecord(int id, Point location): id(id), location(location) {}
};



struct Line {
    double a_;
    double b_;
    double c_;
};

struct Either {
            DirectedArc directedArc;
            DirectedSegment directedSegment;
            bool isArc;
            bool isSegment;
            Either(DirectedArc directedArc): directedArc(directedArc), isArc(true), isSegment(false) {}
            Either(DirectedSegment directedSegment): directedSegment(directedSegment), isArc(false), isSegment(true) {}
            Either() {}

            Point from() {
            if (isArc) {
            return directedArc.from;
            } else {
            return directedSegment.from;
            }
            }
            Point to() {
            if (isArc) {
            return directedArc.to;
            } else {
            return directedSegment.to;
            }
            }


            };

struct Ellipse {
  private:
      double _a, _b, _c, _d, _e, _f;
      double b_, c_;

  public:
      Point f1_, f2_;
      double a_;

      Ellipse() {
          _a = DBL_MIN;
          _b = DBL_MIN;
          _c = DBL_MIN;
          _d = DBL_MIN;
          _e = DBL_MIN;
          _f = DBL_MIN;
          a_ = DBL_MIN;
          b_ = DBL_MIN;
          c_ = DBL_MIN;
      }

      double &a() {
          return a_;
      }
      double b() {
          return b_ != DBL_MIN ? b_ : b_ = sqrt(a() * a() + c() * c());
      }
      double c() {
          return c_ != DBL_MIN ? c_ : c_ = sqrt((f1_.x_ - f2_.x_) * (f1_.x_ - f2_.x_) + (f1_.y_ - f2_.y_) * (f1_.y_ - f2_.y_)) / 2;
      }

  //  double A() { return _a != DBL_MIN ? _a : _a =  (f2_.x_ - f1_.x_) * (f1_.x_ - f2_.x_) / 2 + a() * a(); }
  //  double B() { return _b != DBL_MIN ? _b : _b =  (f2_.y_ - f1_.y_) * (f1_.x_ - f2_.x_); }
  //  double C() { return _c != DBL_MIN ? _c : _c =  (f2_.y_ - f1_.y_) * (f1_.y_ - f2_.y_) / 2 + a() * a(); }
  //  double D() { return _d != DBL_MIN ? _d : _d = -(f2_.x_ + f1_.x_) * A() - (f1_.y_ + f2_.y_) / 2 * B(); }
  //  double E() { return _e != DBL_MIN ? _e : _e = -(f2_.y_ + f1_.y_) * C() - (f1_.x_ + f2_.x_) / 2 * B(); }
  //  double F() { return _f != DBL_MIN ? _f : _f =  (f2_.x_ + f1_.x_) * (f1_.x_ + f2_.x_) / 4 * A() + (f1_.y_ + f2_.y_) * (f1_.y_ + f2_.y_) / 4 * C() + (f1_.x_ + f2_.x_) * (f1_.y_ + f2_.y_) / 4 * B() - 1; }

      double A() {
          return _a != DBL_MIN ? _a : _a = 4 * (f2_.x_ - f1_.x_) * (f2_.x_ - f1_.x_) - 16 * a_ * a_;
      }
      double B() {
          return _b != DBL_MIN ? _b : _b = 8 * (f2_.x_ - f1_.x_) * (f2_.y_ - f1_.y_);
      }
      double C() {
          return _c != DBL_MIN ? _c : _c = 4 * (f2_.y_ - f1_.y_) * (f2_.y_ - f1_.y_) - 16 * a_ * a_;
      }
      double D() {
          return _d != DBL_MIN ? _d : _d = 4 * T() * (f2_.x_ - f1_.x_) + 32 * a_ * a_ * f2_.x_;
      }
      double E() {
          return _e != DBL_MIN ? _e : _e = 4 * T() * (f2_.y_ - f1_.y_) + 32 * a_ * a_ * f2_.y_;
      }
      double F() {
          return _f != DBL_MIN ? _f : _f = T() * T() - 16 * a_ * a_ * (f2_.x_ * f2_.x_ + f2_.y_ * f2_.y_);
      }

      double T() {
          return f1_.x_ * f1_.x_ + f1_.y_ * f1_.y_ - f2_.x_ * f2_.x_ - f2_.y_ * f2_.y_ - 4 * a_ * a_;
      }
};

class G {
  public:
    static std::map<std::tuple<std::vector<Point>, Point, Point>, std::vector<int>> shortestPathCacheInt;
    static std::map<std::tuple<std::vector<Point>, Point, Point>, std::vector<Point>> shortestPathCachePoint;
    static Line line(Point p1, Point p2);
    static Line line(Point *p1, Point *p2) {
      return line(*p1, *p2);
    }
    static Line line(Point *p1, Point p2) {
        return line(*p1, p2);
    }
    static Line line(Point p1, Point *p2) {
        return line(p1, *p2);
    }
    static void line(double x1, double y1, double x2, double y2, double &a, double &b,
        double &c);
    static double distance(Point p1, Point p2);
    static double distance(double x1, double y1, double x2, double y2);
    static double norm(double rad) {
    	while (rad < 0) {
    		rad = rad + (2 * M_PI);
    	}
    	while (rad > (2 * M_PI)) {
    		rad = rad - (2 * M_PI);
    	}
    	return rad;
    }
    static bool is_intersect(Point p1, Point p2, Point p3, Point p4){
      return is_intersect(&p1, &p2, &p3, &p4);
    }
    static bool intersection(Line l1, Line l2, Point &p);
    static bool is_intersect(Point* p1, Point* p2, Point* p3, Point* p4);

    // intersection point of the line a1x + b1y + c1 = 0 and the line a2x + b2y + c2 = 0
    static bool intersection(double a1, double b1, double c1, double a2, double b2,
            double c2, double &x, double &y);

    static bool intersection(Point* p1, Point* p2, Point* p3, Point* p4, Point& p);
    static bool intersection(Point p1, Point p2, Point p3, Point p4, Point& p) {
      return intersection(&p1, &p2, &p3, &p4, p);
    };
    static void centers(Point p1, Point p2, double radius, Point &center1, Point &center2);
    static std::vector<Point> centers(Point p1, Point p2, double radius);
    static Point nearestCenterCCW(Point pivot, Point next, Point center, double radius) {
      Point center1, center2;
      G::centers(pivot, next, radius, center1, center2);

      double pivotAngle = norm(atan2(pivot.y() - center1.y(), pivot.x() - center1.x()));
      double nextAngle = norm(atan2(next.y() - center1.y(), next.x() - center1.x()));
      double diffCCWAngle = norm(pivotAngle - nextAngle);

      if (diffCCWAngle < M_PI) {
        return center1;
      } else {
        return center2;
      }
    }
    static double cross(const Point &O, const Point &A, const Point &B) {
    	return (A.x() - O.x()) * (B.y() - O.y()) - (A.y() - O.y()) * (B.x() - O.x());
    }
    static int findNextHopRollingBall(Point pivot, Point ballCenter, double ballRadius, vector<NeighborRecord> candidates, Point &nextCenter) {
      int nextHop = -1;
      double ballCenterAngle = norm(atan2(ballCenter.y() - pivot.y(), ballCenter.x() - pivot.x()));
      double minAngle = 3 * M_PI;
      for (auto &n: candidates) {
        Point candidateCenter = nearestCenter(pivot, n.location, ballCenter, ballRadius);
        double candidateAngle = norm(atan2(candidateCenter.y() - pivot.y(), candidateCenter.x() - pivot.x()));
        if (norm(candidateAngle - ballCenterAngle) < minAngle) {
          minAngle = norm(candidateAngle - ballCenterAngle);
          nextHop = n.id;
          nextCenter = candidateCenter;
        }
      }

      return nextHop;
    }
    static Point findNextHopRollingBall(Point pivot, Point ballCenter, double ballRadius, std::vector<Point> candidates, Point &nextCenter) {
      Point nextHop;
      double ballCenterAngle = norm(atan2(ballCenter.y() - pivot.y(), ballCenter.x() - pivot.x()));
      double minAngle = 3 * M_PI;
      for (auto &n: candidates) {
        Point candidateCenter = nearestCenter(pivot, n, ballCenter, ballRadius);
        double candidateAngle = norm(atan2(candidateCenter.y() - pivot.y(), candidateCenter.x() - pivot.x()));
        if (norm(candidateAngle - ballCenterAngle) < minAngle) {
          minAngle = norm(candidateAngle - ballCenterAngle);
          nextHop = n;
          nextCenter = candidateCenter;
        }
      }

      return nextHop;
    }

    static Point nearestCenter(Point pivot, Point next, Point center, double ballRadius) {
      Point center1, center2;
      centers(pivot, next, ballRadius, center1, center2);

      double pivotAngle = norm(atan2(pivot.y() - center1.y(), pivot.x() - center1.x()));
      double nextAngle = norm(atan2(next.y() - center1.y(), next.x() - center1.x()));
      double diffCCWAngle = norm(pivotAngle - nextAngle);

      if (diffCCWAngle < M_PI) {
        return center1;
      } else {
        return center2;
      }
    }
    static std::vector<Point> convexHull1(std::vector<Point> P) {
    	int n = P.size(), k = 0;
    	if (n <= 3) return P;
    	std::vector<Point> H(2*n);

    	// Sort points lexicographically
    	sort(P.begin(), P.end());

    	// Build lower hull
    	for (int i = 0; i < n; ++i) {
    		while (k >= 2 && cross(H[k-2], H[k-1], P[i]) <= 0) k--;
    		H[k++] = P[i];
    	}

    	// Build upper hull
    	for (int i = n-1, t = k+1; i >= 0; --i) {
    		while (k >= t && cross(H[k-2], H[k-1], P[i-1]) <= 0) k--;
    		H[k++] = P[i-1];
    	}

    	H.resize(k-1);
    	return H;
    }

    static double polygonPerimeter(std::vector<Point> P) {
      double per = 0;
      int i;
    	for (i=0; i<P.size()-1; i++) {
        per += distance(P[i], P[i+1]);
      }
      per += distance(P[0], P[i]);
      return per;
    }

    static double distanceToLineSegment(Point lp1, Point lp2, Point p) {
      double lx1 = lp1.x(), ly1 = lp1.y();
      double lx2 = lp2.x(), ly2 = lp2.y();
      double px = p.x(), py = p.y();

      double ldx = lx2 - lx1,
             ldy = ly2 - ly1,
             lineLengthSquared = ldx*ldx + ldy*ldy;
      double t;
      if (lineLengthSquared == 0) {
        t = 0;
      } else {
        t = ((px - lx1) * ldx + (py - ly1) * ldy) / lineLengthSquared;
        if (t < 0) t = 0;
        else if (t > 1) t = 1;
      }

       double lx = lx1 + t * ldx,
              ly = ly1 + t * ldy,
              dx = px - lx,
              dy = py - ly;

      return sqrt(dx * dx + dy * dy);
    }

    static bool pointInPolygon(std::vector<Point> vs, Point point);
    static int ccw(Point a, Point b, Point c) {
        int area = (b.x() - a.x()) * (c.y() - a.y()) - (b.y() - a.y()) * (c.x() - a.x());
        if (area > 0)
            return -1;
        else if (area < 0)
            return 1;
        return 0;
    }


    static std::vector<Point> convexHull(std::vector<Point> points)    {
        stack<Point> hull;

        if (points.size() < 3)
            return std::vector<Point>();

        // find the point having the least y coordinate (pivot),
        // ties are broken in favor of lower x coordinate
        int leastY = 0;
        for (int i = 1; i < points.size(); i++)
            if (points[i] < points[leastY])
                leastY = i;

        // swap the pivot with the first point
        Point temp = points[0];
        points[0] = points[leastY];
        points[leastY] = temp;

        // sort the remaining point according to polar order about the pivot
        Point pivot = points[0];
        sort(points.begin() + 1, points.end(), [pivot](Point a, Point b) {
          int order = ccw(pivot, a, b);
          if (order == 0)
              return distance(pivot, a) < distance(pivot, b);
          return (order == -1);
        });

        hull.push(points[0]);
        hull.push(points[1]);
        hull.push(points[2]);

        for (int i = 3; i < points.size(); i++) {
            Point top = hull.top();
            hull.pop();
            while (ccw(hull.top(), top, points[i]) != -1)   {
                top = hull.top();
                hull.pop();
            }
            hull.push(top);
            hull.push(points[i]);
        }

        std::vector<Point> result;
        while (!hull.empty()) {
          Point top = hull.top();
          hull.pop();
          result.push_back(top);
        }
//        reverse(result.begin(), result.end());

        return result;
    }

    static LineSegment translate(LineSegment, Vector);
    static double intersection(LineSegment, Ray);
    static double dot(Vector, Vector);
    static bool onSegment(LineSegment, Point);
    static int orientation(Point p, Point q, Point r);
    static bool doIntersect(LineSegment, LineSegment);
    static bool pointOnPolygon(std::vector<Point>, Point);
    static bool pointInOrOnPolygon(std::vector<Point> vs, Point p);
    static bool pointInOrOnPolygon(std::vector<CavernNode> vs, Point p);
    static bool pointOutOrOnPolygon(std::vector<Point> vs, Point p);
    static std::vector<Point> shortestPathInOrOnPolygon(std::vector<Point> vs, Point source, Point destination);
    static std::vector<int> shortestPathInOrOnPolygon2(std::vector<Point> vs, Point source, Point destination);
    static std::vector<Point> shortestPathInOrOnPolygon(std::vector<CavernNode> vs, Point source, Point destination);
    static std::vector<int> shortestPathInOrOnPolygon2(std::vector<CavernNode> vs, Point source, Point destination);
    static std::vector<Point> shortestPathOutOrOnPolygon(std::vector<Point> vs, Point source, Point destination);
    static std::vector<int> shortestPathOutOrOnPolygon2(std::vector<Point> vs, Point source, Point destination);

    static bool inOrOnPolygon(std::vector<Point>, LineSegment);
    static bool outOrOnPolygon(std::vector<Point>, LineSegment);
    static Point intersection(LineSegment, LineSegment);
    static std::vector<Point> intersectionMarks(LineSegment, LineSegment);
    static bool is_intersect(DirectedSegment, DirectedArc, Point &intersection);
    static bool is_intersect(DirectedArc, DirectedArc, Point &intersection);
    static bool is_intersect(DirectedArc, std::vector<Point> hole);
    static bool is_intersect(LineSegment, std::vector<Point> hole);
    static double acossafe(double x);
    static Point rotatePoint(Point, Point, double angle);
    static bool distanceEqual(Point, Point, double);
    static std::vector<Point> rollBallCavern(std::vector<Point>, double);
    static std::vector<Point> rollBallPolygon(std::vector<Point>, double);
    static void debugLine(double, double, double, double, string color);
    static void debugLine(Point, Point, string color);
    static void debugCircle(double, double, double, string color);
    static void debugCircle(Point, double, string color);
    static void debugPoint(double, double, string color);
    static void debugPoint(Point, string color);
    static void debugArc(Point from, Point to, double radius, string color);
    static void debugPolygon(std::vector<Point>, string color);
    static void debugPath(std::vector<Point>, string color);
	static std::ostream & trace();
	static std::ostream & log();
	static Point closestPointOnSegment(LineSegment, Point);
	static Point closestPointOnPolygon(std::vector<Point>, Point);
	static double distanceToPolygon(std::vector<Point>, Point);
	static double distanceToLineSegment(LineSegment, Point);
	static double deepness(std::vector<Point>);
	static std::vector<Point> flatten(std::vector<Point> path);
	static double diameter(std::vector<Point> polygon);
	static std::vector<Point> rollBallPath(std::vector<Point> path, double r);
  static bool isSameSign(double x, double y);
  static int isMonotoneLine(int a, int b, std::vector<Point> hole);
  static int isMonotoneLine(Point A, Point B, std::vector<Point> hole);
  static int getVertexIndexInPolygon(Point P, std::vector<Point> hole);
  static Point randomPointInPolygon(std::vector<Point>);
	static tuple<Point, Point> hash(std::vector<Point>);

	static std::vector<Point> translate(std::vector<Point> path, double x);


  // check whether 3 points (x1, y1), (x2, y2), (x3, y3) are in same line
    static bool is_in_line(double x1, double y1, double x2, double y2, double x3, double y3);

    // check whether p1, p2, p3 are in same line
    static bool is_in_line(Point p1, Point p2, Point p3) {
        return is_in_line(p1.x_, p1.y_, p2.x_, p2.y_, p3.x_, p3.y_);
    }

    static bool is_in_line(Point *p1, Point *p2, Point *p3) {
        return is_in_line(*p1, *p2, *p3);
    }

    static bool is_in_line(Point *p1, Point *p2, Point p3) {
        return is_in_line(*p1, *p2, p3);
    }

    // check whether x is lies between a, b
    static bool is_between(double x, double a, double b);

    // check whether (x, y) is contained by rectangular x1, y1, x2, y2 (except the boundary) or by segment (x1, y1)(x2, y2) (except the vertex)
    static bool is_between(double x, double y, double x1, double y1, double x2, double y2);

    // check whether p1 is contained by rectangular with p2 and p3 are non-adjacent vertex
    static bool is_between(Point p1, Point p2, Point p3) {
        return is_between(p1.x_, p1.y_, p2.x_, p2.y_, p3.x_, p3.y_);
    }

    // Check whther (x1, y1)(x2, y2) and (x3, y3)(x4, y4) is intersect
    static bool is_intersect(double x1, double y1, double x2, double y2, double x3, double y3, double x4, double y4);

    // side of p to l. 0 : l contain p, <0 side, >0 other side
    static int position(Point *p, Line *l);

    static int position(Point p, Line *l) {
        return position(&p, l);
    }

    static int position(Point *p, Line l) {
        return position(p, &l);
    }

    static int position(Point p, Line l) {
        return position(&p, &l);
    }

    // distance p1 to p2
    static double distance(Point *p1, Point p2) {
        return distance(p1, &p2);
    }

    static double distance(Point p1, Point *p2) {
        return distance(&p1, p2);
    }

    static double distance(Point *p1, Point *p2);
    
    // distance of p and line l
    static double distance(Point *p, Line *l);

    static double distance(Point p, Line *l) {
        return distance(&p, l);
    }

    static double distance(Point *p, Line l) {
        return distance(p, &l);
    }

    static double distance(Point p, Line l) {
        return distance(&p, &l);
    }

    // distance of (x, y) and line ax + by + c = 0
    static double distance(double x, double y, double a, double b, double c);

    // distance of (x0, y0) and line that contains (x1, y1) and (x2, y2)
    static double distance(double x0, double y0, double x1, double y1, double x2, double y2);

    // angle of line l with ox
    static Angle angle(Line *l);

    static Angle angle(Line l) {
        return angle(&l);
    }

    /**
     * angle of vector (p1, p2) with Ox - "theta" in polar coordinate
     * return angle between (-Pi, Pi]
     */
    static Angle angle(Point p1, Point p2);

    static Angle angle(Point *p1, Point p2) {
        return angle(*p1, p2);
    }

    static Angle angle(Point p1, Point *p2) {
        return angle(p1, *p2);
    }

    static Angle angle(Point *p1, Point *p2) {
        return angle(*p1, *p2);
    }

    /**
     * absolute value of angle (p1, p0, p2)
     */
    static Angle angle(Point *p0, Point *p1, Point *p2);

    static Angle angle(Point p0, Point *p1, Point *p2) {
        return angle(&p0, p1, p2);
    }

    static Angle angle(Point p0, Point p1, Point p2) {
        return angle(&p0, &p1, &p2);
    }
    /**
     * angle of vector (p2, p3) to vector (p0, p1)
     */
    static Angle angle(Point p0, Point p1, Point p2, Point p3);

    static Angle angle(Point *p0, Point *p1, Point *p2, Point *p3) {
        return angle(*p0, *p1, *p2, *p3);
    }

    // angle of vector (v2) to vector (v1)
    static Angle angle(Vector v1, Vector v2);

    // angle from segment ((x2, y2), (x0, y0)) to segment ((x1, y1), (x0, y0))
    static Angle angle(double x0, double y0, double x1, double y1, double x2, double y2);

    // Check if segment [p1, p2] is intersect segment [p3, p4]

    static bool is_intersect(Point *p1, Point *p2, Point *p3, Point p4) {
        return is_intersect(p1, p2, p3, &p4);
    }

    static bool is_intersect(Point *p1, Point *p2, Point p3, Point p4) {
        return is_intersect(p1, p2, &p3, &p4);
    }

    static bool is_intersect2(Point *p1, Point *p2, Point *p3, Point *p4);

    static bool is_intersect2(Point *p1, Point *p2, Point p3, Point p4) {
        return is_intersect2(p1, p2, &p3, &p4);
    }
    static bool is_intersect2(Point p1, Point p2, Point p3, Point p4) {
        return is_intersect2(&p1, &p2, &p3, &p4);
    }

    static bool intersection1(Point p1, Point p2, Point p3, Point p4, Point &p);

    // Point that is intersection point of l1 and l2
    static bool intersection(Line l1, Line l2, Point *p) {
        return intersection(l1, l2, *p);
    }

    // intersection point of the line that contains (x1, y1), (x2, y2) and the line that contains (x3, y3), (x4, y4)
    static bool intersection(double x1, double y1, double x2, double y2, double x3, double y3, double x4, double y4, double &x, double &y);

    // Check if ellipse e and line l is intersect
    // static int intersection(Ellipse *e, Line *l, Point &p1, Point &p2);

    // static int intersection(Ellipse e, Line *l, Point &p1, Point &p2) {
    //     return intersection(&e, l, p1, p2);
    // }

    // static int intersection(Ellipse *e, Line l, Point &p1, Point &p2) {
    //     return intersection(e, &l, p1, p2);
    // }

    // static int intersection(Ellipse e, Line l, Point &p1, Point &p2) {
    //     return intersection(&e, &l, p1, p2);
    // }

    // midpoint of p1 and p2
    static Point midpoint(Point *p1, Point *p2) {
        return midpoint(*p1, *p2);
    }

    static Point midpoint(Point p1, Point *p2) {
        return midpoint(p1, *p2);
    }

    static Point midpoint(Point *p1, Point p2) {
        return midpoint(*p1, p2);
    }

    static Point midpoint(Point p1, Point p2);

    // vector P1P2
    static Vector vector(Point *p1, Point *p2) {
        return vector(*p1, *p2);
    }

    static Vector vector(Point *p1, Point p2) {
        return vector(*p1, p2);
    }

    static Vector vector(Point p1, Point *p2) {
        return vector(p1, *p2);
    }

    static Vector vector(Point p1, Point p2);

    // vector with slope k
    static Vector vector(Angle k);

    // line throw p and have slope k
    static Line line(Point *p, Angle k) {
        return line(*p, k);
    }

    static Line line(Point p, Angle k);

    // line contains p and perpendicular with l
    static Line perpendicular_line(Point *p, Line *l);

    static Line perpendicular_line(Point p, Line *l) {
        return perpendicular_line(&p, l);
    }

    static Line perpendicular_line(Point *p, Line l) {
        return perpendicular_line(p, &l);
    }

    static Line perpendicular_line(Point p, Line l) {
        return perpendicular_line(&p, &l);
    }

    // line contains (x0, y0) and perpendicular with the line that contains (x1, y1) and (x2, y2)
    static void perpendicular_line(double x0, double y0, double x1, double y1, double x2, double y2, double &a, double &b, double &c);

    // line contains (x0, y0) and perpendicular with line a0x + b0y + c0 = 0
    static void perpendicular_line(double x0, double y0, double a0, double b0, double c0, double &a, double &b, double &c);

    // line parallel with l and have distance to l of d
    static void parallel_line(Line l, double d, Line &l1, Line &l2);

    // line contains p and parallel with l
    static Line parallel_line(Point *p, Line *l);

    static Line parallel_line(Point p, Line *l) {
        return parallel_line(&p, l);
    }

    static Line parallel_line(Point *p, Line l) {
        return parallel_line(p, &l);
    }

    static Line parallel_line(Point p, Line l) {
        return parallel_line(&p, &l);
    }

    // line contains (x0, y0) and parallel with the line that contains (x1, y1) and (x2, y2)
    static void parallel_line(double x0, double y0, double x1, double y1, double x2, double y2, double &a, double &b, double &c);

    // line that contains (x0, y0) and parallel with line a0x + b0y + c0 = 0
    static void parallel_line(double x0, double y0, double a0, double b0, double c0, double &a, double &b, double &c);

    // the angle bisector line of (p1 p2 p3)
    static Line angle_bisector(Point p1, Point p2, Point p3);

    static Line angle_bisector(Point *p1, Point *p2, Point p3) {
        return angle_bisector(*p1, *p2, p3);
    }

    static Line angle_bisector(Point *p1, Point *p2, Point *p3) {
        return angle_bisector(*p1, *p2, *p3);
    }

    // draw the angle bisector ax + by + c = 0 of angle (x1, y1)(x0, y0)(x2, y2)
    static void angle_bisector(double x0, double y0, double x1, double y1, double x2, double y2, double &a, double &b, double &c);

    // tangent lines of ellipse e that have tangent point p
    // static Line tangent(Ellipse *e, Point *p);

    // static Line tangent(Ellipse e, Point *p) {
    //     return tangent(&e, p);
    // }

    // static Line tangent(Ellipse *e, Point p) {
    //     return tangent(e, &p);
    // }

    // static Line tangent(Ellipse e, Point p) {
    //     return tangent(&e, &p);
    // }

    // tangent lines of circle c that contains p, return tangents line available or not
    static bool tangent(Circle2 *c, Point *p, Line &t1, Line &t2);

    // get tangent points of tangent lines of circle that has center O(a, b) and radius r pass through M(x, y)
    static bool tangent_point(Circle2 *c, Point *p, Point &t1, Point &t2);

    // get tangent points of tangent lines of circle that has center O(a, b) and radius r pass through M(x, y)
    static bool tangent_point(double a, double b, double r, double x, double y, double &t1x, double &t1y, double &t2x, double &t2y);

    // find circumcenter contains p1, p2 and p3
    static Circle2 circumcenter(Point p1, Point p2, Point p3) {
        return circumcenter(&p1, &p2, &p3);
    }

    static Circle2 circumcenter(Point *p1, Point *p2, Point *p3);

    // find circumcenter (xo, yo)
    static void circumcenter(double x1, double y1, double x2, double y2, double x3, double y3, double &xo, double &yo);

    // quadratic equation. return number of experiment
    static int quadratic_equation(double a, double b, double c, double &x1, double &x2);

    // area of Triangle (ax,ay), (bx,by), (cx,cy)
    static double area(double ax, double ay, double bx, double by, double cx, double cy);

    // area of Triangle p1, p2, p3
    static double area(Point p1, Point p2, Point p3) {
        return area(p1.x_, p1.y_, p2.x_, p2.y_, p3.x_, p3.y_);
    }

    static double area(Point *p1, Point *p2, Point *p3) {
        return area(*p1, *p2, *p3);
    }

    // find perpendicular bisector of segment ((x1, y1), (x2, y2))
    static void perpendicular_bisector(double x1, double y1, double x2, double y2, double &a, double &b, double &c);

    // extenstion
    static Angle directedAngle(Point *a, Point *p, Point *b);

    static bool lineSegmentIntersection(Point *a, Point *b, Line l, Point &p);

    static Angle angle_x_axis(Point *a, Point *p);

    static bool onSegment(Point *p, Point *q, Point *r) {
        return onSegment(*p, *q, *r);
    }

    static bool onSegment(Point p, Point q, Point r);

    static int orientation2(Point p, Point q, Point r);

    static bool doIntersect(Point p1, Point q1, Point p2, Point q2);

    static bool isPointLiesInTriangle(Point *p, Point *p1, Point *p2, Point *p3);

    static bool isPointLiesInTriangle(Point p, Point p1, Point p2, Point p3) {
        return isPointLiesInTriangle(&p, &p1, &p2, &p3);
    }

    static int position(Point *p1, Point *p2, Line *l);

    static int circleCircleIntersect(Circle2 c1, Circle2 c2, Point *intersect1, Point *intersect2) {
        return G::circleCircleIntersect(c1, c1.radius_, c2, c2.radius_, intersect1, intersect2);
    }

    static int circleCircleIntersect(Point* c1, double r1, Point* c2, double r2, Point *p1, Point *p2) {
        return circleCircleIntersect(*c1, r1, *c2, r2, p1, p2);
    }

    static int circleCircleIntersect(Point c1, double r1, Point c2, double r2, Point *p1, Point *p2);
    static int circleLineIntersect(Point c, double r, Point p1, Point p2, Point *i1, Point *i2);

    static int segmentAggregation(Point *a1, Point *a2, Point *b1, Point *b2);

    static int orientation(Point* p, Point q, Point* r);

    static Point intersectSections(Point& begin1, Point& end1, Point& begin2, Point& end2);

    static double determinant(double a1, double a2, double b1, double b2);

    static double getVectorAngle(Point p1, Point p2);

    static bool isPointInsidePolygon(Point p, std::vector<Point> polygon);

    static bool isPointReallyInsidePolygon(Point p, std::vector<Point> polygon);

    static bool segmentPolygonIntersect(Point, Point, std::vector<Point>);

    static bool segmentPolygonIntersect(Point, Point, std::vector<Point>, Point &);

    static bool segmentPolygonIntersect1(Point, Point, std::vector<Point>);

    static bool isSegmentInsidePolygon(Point, Point, std::vector<Point>);

    static bool isSegmentInsidePolygon1(Point, Point, std::vector<Point>);

    static bool inSegment(Point p, Point a, Point b);

  private:
    static int circleCircleIntersect0a(double r1, Point c2, double r2, Point *p1, Point *p2);

    static int circleCircleIntersect0b(double r1, Point c2, double r2, Point *p1, Point *p2);

    static void circleCircleIntersect00(double r1, double a2, double r2, Point *p1, Point *p2);

    static int circleLineIntersect00(double r, Point i1, Point i2, Point *p1, Point *p2);
};


#endif /* GEOMETRY_GEOMATHHELPER_H_ */
