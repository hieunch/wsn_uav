#include "GeoMathHelper.h"
#include "DebugInfoWriter.h"
#include <cmath>

std::ostream & G::log() {
  return trace() << " WSN_LOG ";
}

std::map<std::tuple<std::vector<Point>, Point, Point>, std::vector<int>> G::shortestPathCacheInt;
std::map<std::tuple<std::vector<Point>, Point, Point>, std::vector<Point>> G::shortestPathCachePoint;

void G::debugLine(double x1, double y1, double x2, double y2, string color) {
  trace() << "WSN_EVENT DRAW LINE " << x1 << " " << y1 << " " << x2 << " " << y2 << " " << color;
}
void G::debugLine(Point p1, Point p2, string color) {
  debugLine(p1.x(), p1.y(), p2.x(), p2.y(), color);
}
void G::debugCircle(double centerX, double centerY, double radius, string color) {
  trace() << "WSN_EVENT DRAW CIRCLE " << centerX << " " << centerY << " " << radius << " " << color;
}
void G::debugCircle(Point center, double radius, string color) {
  debugCircle(center.x(), center.y(), radius, color);
}
void G::debugPoint(double x1, double x2, string color) {
  trace() << "WSN_EVENT DRAW POINT " << x1 << " " << x2 << " " << color;
}
void G::debugPoint(Point p, string color) {
  debugPoint(p.x(), p.y(), color);
}
void G::debugArc(Point from, Point to, double radius, string color) {
  trace() << "WSN_EVENT DRAW ARC " << from.x() << " " << from.y() << " " << to.x() << " "
    << to.y() << " " << radius << " " << color;
}

std::ostream & G::trace() {
  return (ostream &) DebugInfoWriter::getStream() << endl << " ";
}
void G::debugPolygon(std::vector<Point> ps, string color) {
  for (int i = 0; i < ps.size(); i++) {
    Point from = ps[i];
    Point to = ps[(i + 1) % ps.size()];
    debugLine(from, to, color);
  }
}

void G::debugPath(std::vector<Point> ps, string color) {
  for (int i = 0; i < ps.size() - 1; i++) {
    Point from = ps[i];
    Point to = ps[(i + 1) % ps.size()];
    debugLine(from, to, color);
  }
}

std::ostream &operator<<(std::ostream &os, const Point &m) {
    return os << "Point(" << m.x() << ", " << m.y() << ")";
}

std::ostream &operator<<(std::ostream &os, const Vector &v) {
    return os << "Vector(" << v.x << ", " << v.y << ")";
}

Line G::line(Point p1, Point p2) {
    Line re;
    line(p1.x_, p1.y_, p2.x_, p2.y_, re.a_, re.b_, re.c_);
    return re;
}

void G::line(double x1, double y1, double x2, double y2, double &a, double &b,
        double &c) {
    a = y1 - y2;
    b = x2 - x1;
    c = -y1 * x2 + y2 * x1;
}

double G::distance(Point p1, Point p2) {
    return distance(p1.x_, p1.y_, p2.x_, p2.y_);
}
double G::distance(double x1, double y1, double x2, double y2) {
    return sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
}

double G::distance(Point *p1, Point *p2) {
        return distance(p1->x_, p1->y_, p2->x_, p2->y_);
    }

// distance of p and line l
    double G::distance(Point *p, Line *l) {
        return distance(p->x_, p->y_, l->a_, l->b_, l->c_);
    }

    double G::distance(double x0, double y0, double x1, double y1, double x2,
                       double y2) {
        double a, b, c;
        line(x1, y1, x2, y2, a, b, c);
        return distance(x0, y0, a, b, c);
    }

    double G::distance(double x, double y, double a, double b, double c) {
        return fabs(a * x + b * y + c) / sqrt(a * a + b * b);
    }

// side of p to l. 0 : l contain p, <0 side, >0 other side
    int G::position(Point *p, Line *l) {
        double temp = l->a_ * p->x_ + l->b_ * p->y_ + l->c_;
        if (fabs(temp) < EPSILON)
            temp = 0;
        return temp ? (temp > 0 ? 1 : -1) : 0;
    }

// side of p1 to p2 through l: >0 if same side, =0 if either p1 or p2 stay on l1, <0 if difference side
    int G::position(Point *p1, Point *p2, Line *l) {
        int pos1 = G::position(p1, l);
        int pos2 = G::position(p2, l);

        return pos1 * pos2;
    }

// Check if segment [p1, p2] and segment [p3, p4] are intersected
bool G::is_intersect(Point* p1, Point* p2, Point* p3, Point* p4) {
  Line l1 = line(p1, p2);
  Line l2 = line(p3, p4);
  Point in;
  return (intersection(l1, l2, in)
          && ((in.x_ - p1->x_) * (in.x_ - p2->x_) <= 0)
          && ((in.y_ - p1->y_) * (in.y_ - p2->y_) <= 0)
          && ((in.x_ - p3->x_) * (in.x_ - p4->x_) <= 0)
          && ((in.y_ - p3->y_) * (in.y_ - p4->y_) <= 0));
}

// Check if segment [p1, p2] and segment [p3, p4] are intersected
bool G::intersection(Point* p1, Point* p2, Point* p3, Point* p4, Point& p) {
    Line l1 = line(p1, p2);
    Line l2 = line(p3, p4);
    return (intersection(l1, l2, p) && ((p.x_ - p1->x_) * (p.x_ - p2->x_) <= 0)
            && ((p.y_ - p1->y_) * (p.y_ - p2->y_) <= 0)
            && ((p.x_ - p3->x_) * (p.x_ - p4->x_) <= 0)
            && ((p.y_ - p3->y_) * (p.y_ - p4->y_) <= 0));
}

bool G::intersection(Line l1, Line l2, Point& p) {
  return intersection(l1.a_, l1.b_, l1.c_, l2.a_, l2.b_, l2.c_, p.x_, p.y_);
}


bool G::intersection(double a1, double b1, double c1, double a2, double b2,
        double c2, double &x, double &y) {
    if (a1 == 0 && b1 == 0)
        return false;
    if (a2 == 0 && b2 == 0)
        return false;

    if (a1 == 0 && b2 == 0) {
        x = -c2 / a2;
        y = -c1 / b1;
    } else if (a2 == 0 && b1 == 0) {
        x = -c1 / a1;
        y = -c2 / b2;
    } else if (a1 * b2 != a2 * b1) {
        x = (b1 * c2 - b2 * c1) / (a1 * b2 - a2 * b1);
        y = (c1 * a2 - c2 * a1) / (a1 * b2 - a2 * b1);
    } else
        return a1 * c2 == a2 * c1;

    return true;
}

// find center of the circle with radius on which two points are located
void G::centers(Point p1, Point p2, double r, Point &center1, Point &center2) {
  double x1 = p1.x(), y1 = p1.y();
  double x2 = p2.x(), y2 = p2.y();
  double x3 = (x1 + x2) / 2;
  double y3 = (y1 + y2) / 2;
  double q = distance(p1, p2);

  center1 = Point(
    x3 + sqrt(r * r - (q / 2) * (q / 2)) * (y1 - y2) / q,
    y3 + sqrt(r * r - (q / 2) * (q / 2)) * (x2 - x1) / q
  );

  center2 = Point(
    x3 - sqrt(r * r - (q / 2) * (q / 2)) * (y1 - y2) / q,
    y3 - sqrt(r * r - (q / 2) * (q / 2)) * (x2 - x1) / q
  );
}

LineSegment G::translate(LineSegment ls, Vector v) {
  return LineSegment(ls.u + v, ls.v + v);
}

std::vector<Point> G::centers(Point p1, Point p2, double radius) {
  Point center1, center2;
  centers(p1, p2, radius, center1, center2);

  return {center1, center2};
}

Point Point::operator+(Vector v) {
  return Point(x() + v.x, y() + v.y);
}

double G::dot(Vector u, Vector v) {
  return u.x * v.x + u.y * v.y;
}

double G::intersection(LineSegment ls, Ray r) {
  Vector v1 = Vector(ls.u, r.p);
  Vector v2 = Vector(ls.u, ls.v);
  Vector v3 = Vector(-r.v.y, r.v.x);


  double dt = dot(v2, v3);
  if (abs(dt) < 0.0000001) {
    return -1;
  }
  double t1 = v2.crossProduct(v1) / dt;
  double t2 = dot(v1, v3) / dt;

  if (t1 >= 0.0 && (t2 >= 0.0 && t2 <= 1.0))
    return t1;
  return -1;
}
bool G::pointInPolygon(std::vector<Point> vs, Point point) {
  for (int i = 0; i < vs.size(); i++) {
    Point a = vs[i];
    Point b = vs[(i + 1) % vs.size()];
    if (onSegment(LineSegment(a, b), point)) {
      return true;
    }
  }

  double x = point.x(), y = point.y();
  bool inside = false;

  for (int i = 0, j = vs.size() - 1; i < vs.size(); j = i++) {
    double xi = vs[i].x(), yi = vs[i].y();
    double xj = vs[j].x(), yj = vs[j].y();

    double intersect = ((yi > y) != (yj > y))
                && (x < (xj - xi) * (y - yi) / (yj - yi) + xi);
    if (intersect) inside = !inside;
  }

  return inside;
}
bool G::onSegment(LineSegment ls, Point p) {
  Vector uv(ls.u, ls.v);
  Vector up(ls.u, p);
  Vector pv(p, ls.v);

  return abs(uv.length() - up.length() - pv.length()) < EPSILON;
}
int G::orientation(Point p, Point q, Point r) {
  // See https://www.geeksforgeeks.org/orientation-3-ordered-points/
  // for details of below formula.
  int val = (q.y() - p.y()) * (r.x() - q.x()) -
            (q.x() - p.x()) * (r.y() - q.y());

  if (val == 0) return 0;  // colinear

  return (val > 0) ? 1 : 2; // clock or counterclock wise
}
bool G::doIntersect(LineSegment ls1, LineSegment ls2) {
  Point p1 = ls1.u, q1 = ls1.v;
  Point p2 = ls2.u, q2 = ls2.v;

  // Find the four orientations needed for general and
  // special cases
  int o1 = orientation(p1, q1, p2);
  int o2 = orientation(p1, q1, q2);
  int o3 = orientation(p2, q2, p1);
  int o4 = orientation(p2, q2, q1);

  // General case
  if (o1 != o2 && o3 != o4)
      return true;

  // Special Cases
  // p1, q1 and p2 are colinear and p2 lies on segment p1q1
  if (o1 == 0 && onSegment(LineSegment(p1, q1), p2)) return true;

  // p1, q1 and q2 are colinear and q2 lies on segment p1q1
  if (o2 == 0 && onSegment(LineSegment(p1, q1), q2)) return true;

  // p2, q2 and p1 are colinear and p1 lies on segment p2q2
  if (o3 == 0 && onSegment(LineSegment(p2, q2), p1)) return true;

   // p2, q2 and q1 are colinear and q1 lies on segment p2q2
  if (o4 == 0 && onSegment(LineSegment(p2, q2), q1)) return true;

  return false; // Doesn't fall in any of the above cases
}

bool G::pointOnPolygon(std::vector<Point> points, Point p) {
  for (int i = 0; i < points.size(); i++) {
    Point a = points[i];
    Point b = points[(i + 1) % points.size()];
    LineSegment ab(a, b);
    if (onSegment(ab, p)) return true;
  }

  return false;
}

bool G::pointInOrOnPolygon(std::vector<Point> vs, Point p) {
  return pointInPolygon(vs, p);
}
bool G::pointInOrOnPolygon(std::vector<CavernNode> vs, Point p) {
  std::vector<Point> new_vs;
  for (Point p : vs) new_vs.push_back(p);
  return pointInPolygon(new_vs, p);
}
bool G::pointOutOrOnPolygon(std::vector<Point> vs, Point p) {
  if (pointOnPolygon(vs, p)) return true;
  return (!pointInPolygon(vs, p));
}

std::vector<Point> G::shortestPathInOrOnPolygon(std::vector<Point> vs, Point source, Point destination) {
  if (shortestPathCachePoint.find(make_tuple(vs, source, destination)) != shortestPathCachePoint.end()) {
    return shortestPathCachePoint[make_tuple(vs, source, destination)];
  }
  std::vector<Point> points;
  points.push_back(source);
  for (auto p: vs) points.push_back(p);
  points.push_back(destination);
  int V = vs.size() + 2;

  // find shortest path from 0 to V - 1;
  int trace[V + 2];
  trace[0] = -1;

  double d[V + 2];
  d[0] = 0;
  for (int i = 1; i < V; i++) {
    d[i] = 1e9;
  }

  std::set<pair<double, int> > st;
  for (int i = 0; i < V; i++) st.insert({d[i], i});

  while (!st.empty()) {
    auto pr = st.begin();
    int id; double dis;
    tie(dis, id) = *pr;
    st.erase(pr);

    if (id == V - 1) break;

    for (int i = 0; i < V; i++) {
      // if (i != id && inOrOnPolygon(vs, LineSegment(points[id], points[i]))) {
      if (i != id) {
        if (d[id] + distance(points[id], points[i]) < d[i]) {
          auto it = st.find({d[i], i});
          if (it != st.end()) st.erase(it);

          d[i] = d[id] + distance(points[id], points[i]);
          st.insert({d[i], i});
          trace[i] = id;
        }
      }
    }
  }

  std::vector<Point> result;
  int current = V - 1;
  while (current != -1) {
    result.push_back(points[current]);
    current = trace[current];
  }

  reverse(result.begin(), result.end());

  return result;
}

std::vector<int> G::shortestPathInOrOnPolygon2(std::vector<Point> vs, Point source, Point destination) {
  if (shortestPathCacheInt.find(make_tuple(vs, source, destination)) != shortestPathCacheInt.end()) {
    return shortestPathCacheInt[make_tuple(vs, source, destination)];
  }

  int srcId = -1;
  int desId = -1;
  for (int i=0; i<vs.size(); i++) {
    if (vs[i] == source){
      srcId = i;
      break;
    }
  }
  for (int i=0; i<vs.size(); i++) {
    if (vs[i] == destination){
      desId = i;
      break;
    }
  }

  std::vector<Point> points;
  points.push_back(source);
  for (auto p: vs) points.push_back(p);
  points.push_back(destination);
  int V = vs.size() + 2;

  // find shortest path from 0 to V - 1;
  int trace[V + 2];
  trace[0] = -1;

  double d[V + 2];
  d[0] = 0;
  for (int i = 1; i < V; i++) {
    d[i] = 1e9;
  }

  std::set<pair<double, int> > st;
  for (int i = 0; i < V; i++) st.insert({d[i], i});

  while (!st.empty()) {
    auto pr = st.begin();
    int id; double dis;
    tie(dis, id) = *pr;
    st.erase(pr);

    if (id == V - 1) break;

    for (int i = 0; i < V; i++) {
      // if (i != id && inOrOnPolygon(vs, LineSegment(points[id], points[i]))) {
      if (i != id) {
        if (d[id] + distance(points[id], points[i]) < d[i]) {
          auto it = st.find({d[i], i});
          if (it != st.end()) st.erase(it);

          d[i] = d[id] + distance(points[id], points[i]);
          st.insert({d[i], i});
          trace[i] = id;
        }
      }
    }
  }

  std::vector<int> result;
  if (desId > 0) result.push_back(desId);
  int current = V - 1;
  while (current != -1) {
    if ((current != 0) && (current != V-1)){
      result.push_back(current-1);
    }
    current = trace[current];
  }
  if (srcId > 0) result.push_back(srcId);

  reverse(result.begin(), result.end());

  return result;
}

// std::vector<Point> G::shortestPathInOrOnPolygon3(std::vector<Point> vs, Point source, Point destination) {
//   if (shortestPathCachePoint.find(make_tuple(vs, source, destination)) != shortestPathCachePoint.end()) {
//     return shortestPathCachePoint[make_tuple(vs, source, destination)];
//   }
//   std::vector<Point> points;
//   points.push_back(source);
//   for (auto p: vs) points.push_back(p);
//   points.push_back(destination);

//   vector<Point> result;

//   DGraph *graph = new DGraph(points);
//   result = graph->shortestPath(source, destination);

//   return result;
// }

std::vector<Point> G::shortestPathInOrOnPolygon(std::vector<CavernNode> vs, Point source, Point destination) {
  std::vector<Point> new_vs;
  for (Point p : vs) new_vs.push_back(p);
  return shortestPathInOrOnPolygon(new_vs, source, destination);
}

std::vector<int> G::shortestPathInOrOnPolygon2(std::vector<CavernNode> vs, Point source, Point destination) {
  std::vector<Point> new_vs;
  for (Point p : vs) new_vs.push_back(p);
  return shortestPathInOrOnPolygon2(new_vs, source, destination);
}

std::vector<int> G::shortestPathOutOrOnPolygon2(std::vector<Point> vs, Point source, Point destination) {
  if (shortestPathCacheInt.find(make_tuple(vs, source, destination)) != shortestPathCacheInt.end()) {
    return shortestPathCacheInt[make_tuple(vs, source, destination)];
  }

  int srcId = -1;
  int desId = -1;
  for (int i=0; i<vs.size(); i++) {
    if (vs[i] == source){
      srcId = i;
      break;
    }
  }
  for (int i=0; i<vs.size(); i++) {
    if (vs[i] == destination){
      desId = i;
      break;
    }
  }

  std::vector<Point> points;
  points.push_back(source);
  for (auto p: vs) points.push_back(p);
  points.push_back(destination);
  int V = vs.size() + 2;

  // find shortest path from 0 to V - 1;
  int trace[V + 2];
  trace[0] = -1;

  double d[V + 2];
  d[0] = 0;
  for (int i = 1; i < V; i++) {
    d[i] = 1e9;
  }

  std::set<pair<double, int> > st;
  for (int i = 0; i < V; i++) st.insert({d[i], i});

  while (!st.empty()) {
    auto pr = st.begin();
    int id; double dis;
    tie(dis, id) = *pr;
    st.erase(pr);

    if (id == V - 1) break;

    for (int i = 0; i < V; i++) {
      if (i != id && outOrOnPolygon(vs, LineSegment(points[id], points[i]))) {
      // if (i != id) {
        if (d[id] + distance(points[id], points[i]) < d[i]) {
          auto it = st.find({d[i], i});
          if (it != st.end()) st.erase(it);

          d[i] = d[id] + distance(points[id], points[i]);
          st.insert({d[i], i});
          trace[i] = id;
        }
      }
    }
  }

  std::vector<int> result;
  if (desId > 0) result.push_back(desId);
  int current = V - 1;
  while (current != -1) {
    if ((current != 0) && (current != V-1)){
      result.push_back(current-1);
    }
    current = trace[current];
  }
  if (srcId > 0) result.push_back(srcId);

  reverse(result.begin(), result.end());

  return result;
}

std::vector<Point> G::shortestPathOutOrOnPolygon(std::vector<Point> vs, Point source, Point destination) {
  if (shortestPathCachePoint.find(make_tuple(vs, source, destination)) != shortestPathCachePoint.end()) {
    return shortestPathCachePoint[make_tuple(vs, source, destination)];
  }

  std::vector<Point> points;
  points.push_back(source);
  for (auto p: vs) points.push_back(p);
  points.push_back(destination);
  int V = vs.size() + 2;

  // find shortest path from 0 to V - 1;
  int trace[V + 2];
  trace[0] = -1;

  double d[V + 2];
  d[0] = 0;
  for (int i = 1; i < V; i++) {
    d[i] = 1e9;
  }

  std::set<pair<double, int> > st;
  for (int i = 0; i < V; i++) st.insert({d[i], i});

  while (!st.empty()) {
    auto pr = st.begin();
    int id; double dis;
    tie(dis, id) = *pr;
    st.erase(pr);

    if (id == V - 1) break;

    for (int i = 0; i < V; i++) {
      if (i != id && outOrOnPolygon(vs, LineSegment(points[id], points[i]))) {
        if (d[id] + distance(points[id], points[i]) < d[i]) {
          auto it = st.find({d[i], i});
          if (it != st.end()) st.erase(it);

          d[i] = d[id] + distance(points[id], points[i]);
          st.insert({d[i], i});
          trace[i] = id;
        }
      }
    }
  }

  std::vector<Point> result;
  int current = V - 1;
  while (current != -1) {
    result.push_back(points[current]);
    current = trace[current];
  }

  reverse(result.begin(), result.end());

  return result;
}

// std::vector<Point> G::shortestPathOutOrOnPolygon3(std::vector<Point> vs, Point source, Point destination) {
//   if (shortestPathCachePoint.find(make_tuple(vs, source, destination)) != shortestPathCachePoint.end()) {
//     return shortestPathCachePoint[make_tuple(vs, source, destination)];
//   }

//   std::vector<Point> points;
//   points.push_back(source);
//   for (auto p: vs) points.push_back(p);
//   points.push_back(destination);
  

//   return result;
// }

bool G::inOrOnPolygon(std::vector<Point> vs, LineSegment ls) {
  Point u = ls.u, v = ls.v;
  bool intersected = false;

  for (int i = 0; i < vs.size(); i++) {
    Point a = vs[i];
    Point b = vs[(i + 1) % vs.size()];
    LineSegment ab(a, b);
    if (a == u && b == v) {
      return true;
    }
    if (b == u && a == v) {
      return true;
    }
    if (doIntersect(ab, ls) && (orientation(a, b, u) != 0 && orientation(a, b, v) != 0)) {
      Point X = intersection(ab, ls);
      if (u != X && v != X && a != X && b != X) return false;
    }
    if (onSegment(ab, u) && onSegment(ab, v)) {
      return true;
    }
    if (doIntersect(ab, ls)) intersected = true;
  }

  if (!intersected) return true;

  std::vector<pair<double, Point>> marks;
  marks.push_back({1, v});
  marks.push_back({0, u});
  for (int i = 0; i < vs.size(); i++) {
    Point a = vs[i];
    Point b = vs[(i + 1) % vs.size()];
    LineSegment ab(a, b);
    if (doIntersect(ab, ls)) {
      for (auto p: intersectionMarks(ab, ls)) {
        double k = Vector(u, p).length() / Vector(u, v).length();
        marks.push_back({k, v});
      }
    }
  }

  std::sort(marks.begin(), marks.end());
  for (int i = 0; i < marks.size() - 1; i++) {
    Point X = marks[i].second;
    Point Y = marks[i + 1].second;
    Point M = Point((X.x() + Y.x()) / 2, (X.y() + Y.y()) / 2);

    if (!pointInOrOnPolygon(vs, M)) return false;
  }

  return true;
}
bool G::outOrOnPolygon(std::vector<Point> vs, LineSegment ls) {
  Point u = ls.u, v = ls.v;
  bool intersected = false;

  for (int i = 0; i < vs.size(); i++) {
    Point a = vs[i];
    Point b = vs[(i + 1) % vs.size()];
    LineSegment ab(a, b);
    if (a == u && b == v) {
      return true;
    }
    if (b == u && a == v) {
      return true;
    }
    if (onSegment(ab, u) && onSegment(ab, v)) {
      return true;
    }

    if (doIntersect(ab, ls) && (orientation(a, b, u) != 0 && orientation(a, b, v) != 0)) {
      Point X = intersection(ab, ls);
      if (u != X && v != X && a != X && b != X) {
        return false;
      }
    }
    if (doIntersect(ab, ls)) intersected = true;
  }

  if (!intersected) return true;

  std::vector<pair<double, Point>> marks;
  marks.push_back({1, v});
  marks.push_back({0, u});
  for (int i = 0; i < vs.size(); i++) {
    Point a = vs[i];
    Point b = vs[(i + 1) % vs.size()];
    LineSegment ab(a, b);
    if (doIntersect(ab, ls)) {
      for (auto p: intersectionMarks(ab, ls)) {
        double k = Vector(u, p).length() / Vector(u, v).length();
        marks.push_back({k, v});
      }
    }
  }

  std::sort(marks.begin(), marks.end());
  for (int i = 0; i < marks.size() - 1; i++) {
    Point X = marks[i].second;
    Point Y = marks[i + 1].second;
    Point M = Point((X.x() + Y.x()) / 2, (X.y() + Y.y()) / 2);

    if (!pointOutOrOnPolygon(vs, M)) return false;
  }

  return true;
}


Point G::intersection(LineSegment ls1, LineSegment ls2) {
  Point result;
  G::intersection(ls1.u, ls1.v, ls2.u, ls2.v, result);

  return result;
}

std::vector<Point> G::intersectionMarks(LineSegment ab, LineSegment uv) {
  // condition: ls1 and ls2 do intersect
  Point a = ab.u, b = ab.v;
  Point u = uv.u, v = uv.v;

  if (onSegment(ab, u) && onSegment(ab, v)) return {u, v};
  if (onSegment(uv, a) && onSegment(uv, b)) return {a, b};

  if (orientation(a, b, u) == 0 && orientation(a, b, v) == 0) {
    if (onSegment(ab, u)) {
      if (onSegment(uv, b)) {
        return {u, b};
      } else {
        return {a, u};
      }
    } else {
      // on segment (ab, v)
      if (onSegment(uv, b)) {
        return {v, b};
      } else {
        return {a, v};
      }
    }
  }


  return {intersection(ab, uv)};
}
bool G::is_intersect(DirectedSegment seg, DirectedArc arc, Point &intersection) {
  if (seg.to == arc.from)  {
    intersection = seg.to;
    return true;
  }

  if (arc.to == seg.from) {
    intersection = seg.from;
    return true;
  }

  if (abs(distance(arc.center, seg.from) - arc.radius) < EPSILON) {
    intersection = seg.from;
    if (Vector(arc.from, intersection).crossProduct(Vector(intersection, arc.to)) <= 0) {
      return true;
    }
  }

  if (abs(distance(arc.center, seg.to) - arc.radius) < EPSILON) {
    intersection = seg.to;
    if (Vector(arc.from, intersection).crossProduct(Vector(intersection, arc.to)) <= 0) {
      return true;
    }
  }


  double dx, dy, A, B, C, det, t;
  Point point1 = seg.from;
  Point point2 = seg.to;
  double radius = arc.radius;
  double cx = arc.center.x();
  double cy = arc.center.y();

  dx = point2.x() - point1.x();
  dy = point2.y() - point1.y();

  A = dx * dx + dy * dy;
  B = 2 * (dx * (point1.x() - cx) + dy * (point1.y() - cy));
  C = (point1.x() - cx) * (point1.x() - cx) +
      (point1.y() - cy) * (point1.y() - cy) -
      radius * radius;

  det = B * B - 4 * A * C;
  if ((A <= 0.0000001) || (det < 0)) {
    return false;
  }
  else if (det == 0) {
    // One solution.
    t = -B / (2 * A);
    if (t >= 0 && t <= 1) {
      intersection = Point(point1.x() + t * dx, point1.y() + t * dy);
      if (Vector(arc.from, intersection).crossProduct(Vector(intersection, arc.to)) <= 0) {
        return true;
      }
    }

    return false;
  }
  else
  {
    // Two solutions.
    t = (double)((-B + sqrt(det)) / (2 * A));
    if (t >= 0 && t <= 1) {
      intersection = Point(point1.x() + t * dx, point1.y() + t * dy);
      if (Vector(arc.from, intersection).crossProduct(Vector(intersection, arc.to)) <= 0) {
        return true;
      }
    }

    t = (double)((-B - sqrt(det)) / (2 * A));
    if (t >= 0 && t <= 1) {
      intersection = Point(point1.x() + t * dx, point1.y() + t * dy);
      if (Vector(arc.from, intersection).crossProduct(Vector(intersection, arc.to)) <= 0) {
        return true;
      }
    }

    return false;
  }

}
bool G::is_intersect(DirectedArc c1, DirectedArc c2, Point &intersection) {
  double r, R, d, dx, dy, cx, cy, Cx, Cy;

  if (c1.radius < c2.radius) {
    r  = c1.radius;  R = c2.radius;
    cx = c1.center.x(); cy = c1.center.y();
    Cx = c2.center.x(); Cy = c2.center.y();
  } else {
    r  = c2.radius; R  = c1.radius;
    Cx = c1.center.x(); Cy = c1.center.y();
    cx = c2.center.x(); cy = c2.center.y();
  }

  // Compute the vector <dx, dy>
  dx = cx - Cx;
  dy = cy - Cy;

  // Find the distance between two points.
  d = sqrt( dx*dx + dy*dy );

  // There are an infinite number of solutions
  // Seems appropriate to also return null
  if (d < EPSILON && abs(R-r) < EPSILON) return false;

  // No intersection (circles centered at the
  // same place with different size)
  else if (d < EPSILON) return false;

  double x = (dx / d) * R + Cx;
  double y = (dy / d) * R + Cy;
  Point P = Point(x, y);

  // Single intersection (kissing circles)
  if (abs((R+r)-d) < EPSILON || abs(R-(r+d)) < EPSILON) {
    // deal with P
    intersection = P;
    if (Vector(c1.from, intersection).crossProduct(Vector(intersection, c1.to)) <= 0) {
      if (Vector(c2.from, intersection).crossProduct(Vector(intersection, c2.to)) <= 0) {
        return true;
      }
    }
  }

  // No intersection. Either the small circle contained within
  // big circle or circles are simply disjoint.
  if ( (d+r) < R || (R+r < d) ) return false;

  Point C = Point(Cx, Cy);
  double angle = acossafe((r*r-d*d-R*R)/(-2.0*d*R));

  intersection = rotatePoint(C, P, +angle);
  if (Vector(c1.from, intersection).crossProduct(Vector(intersection, c1.to)) <= 0) {
    if (Vector(c2.from, intersection).crossProduct(Vector(intersection, c2.to)) <= 0) {
      return true;
    }
  }

  intersection = rotatePoint(C, P, -angle);
  if (Vector(c1.from, intersection).crossProduct(Vector(intersection, c1.to)) <= 0) {
    if (Vector(c2.from, intersection).crossProduct(Vector(intersection, c2.to)) <= 0) {
      return true;
    }
  }

  return false;
}

double G::acossafe(double x) {
  if (x >= +1.0) return 0;
  if (x <= -1.0) return M_PI;
  return acos(x);
}

Point G::rotatePoint(Point fp, Point pt, double a) {
  double x = pt.x() - fp.x();
  double y = pt.y() - fp.y();
  double xRot = x * cos(a) + y * sin(a);
  double yRot = y * cos(a) - x * sin(a);
  return Point(fp.x() + xRot,fp.y() + yRot);
}

bool G::distanceEqual(Point A, Point B, double dis) {
  return abs(distance(A, B) - dis) < EPSILON;
}

std::vector<Point> G::rollBallPath(std::vector<Point> points, double r) {
  std::vector<Either> parts;

  for (int i = 0; i < points.size() - 1; i++) {
    Point A = points[i];
    Point B = points[i + 1];
    Vector AB(A, B);
    Point A1 = A + AB.rotate(M_PI / 2) * (r / AB.length());
    Point B1 = B + AB.rotate(M_PI / 2) * (r / AB.length());
    DirectedSegment A1B1(A1, B1);

    parts.push_back(Either(A1B1));
    Point B2 = B + AB.rotate(-M_PI / 2) * (r / AB.length());
    DirectedArc B1B2(B1, B2, B, r);
    parts.push_back(Either(B1B2));
  }
  parts.pop_back();

  std::vector<Point> trajectory;
  trajectory.push_back(parts[0].from());
  Either current = parts[0];
  int currentId = 0;
  int nextId = 0;
  while (currentId < parts.size()) {
    Either next;
    bool foundNext = false;
    double minDistance = 1e9;

    if (current.isSegment) {
      DirectedSegment currentSegment = current.directedSegment;
      for (int i = currentId + 1; i < parts.size(); i++) {
        Either other = parts[i];
        if (other.isSegment) {
          DirectedSegment otherSegment = other.directedSegment;
          LineSegment ab(currentSegment.from, currentSegment.to),
                      xy(otherSegment.from, otherSegment.to);
          if (G::doIntersect(ab, xy)) {
            foundNext = true;
            Point M;
            if (currentSegment.to == otherSegment.from) {
              M = currentSegment.to;
            } else {
              M = G::intersection(ab, xy);
            }

            if (G::distance(currentSegment.from, M) < minDistance) {
              minDistance = G::distance(currentSegment.from, M);
              next = Either(DirectedSegment(M, otherSegment.to));
              nextId = i;
            }
          }
        } else if (other.isArc) {
          DirectedArc otherArc = other.directedArc;
          Point intersection;
          if (G::is_intersect(currentSegment, otherArc, intersection)) {
            foundNext = true;
            if (G::distance(currentSegment.from, intersection) < minDistance) {
              minDistance = G::distance(currentSegment.from, intersection);
              next = Either(DirectedArc(intersection, otherArc.to, otherArc.center, otherArc.radius));
              nextId = i;
            }
          }
        }
      }

//      if (foundNext)
//        debugLine(current.from(), next.from(), "green");
    } else {
      DirectedArc currentArc = current.directedArc;
      for (int i = currentId + 1; i < parts.size(); i++) {
        Either other = parts[i];
        if (other.isSegment) {
          DirectedSegment otherSegment = other.directedSegment;
          Point intersection;
          if (G::is_intersect(otherSegment, currentArc, intersection)) {
            foundNext = true;
            if (G::distance(currentArc.from, intersection) < minDistance) {
              minDistance = G::distance(currentArc.from, intersection);
              next = Either(DirectedSegment(intersection, otherSegment.to));
              nextId = i;
            }
          }
        } else if (other.isArc) {
          DirectedArc otherArc = other.directedArc;
          Point intersection;
          if (G::is_intersect(currentArc, otherArc, intersection)) {
            foundNext = true;
            if (G::distance(currentArc.from, intersection) < minDistance) {
              minDistance = G::distance(currentArc.from, intersection);
              next = Either(DirectedArc(intersection, otherArc.to, otherArc.center, otherArc.radius));
              nextId = i;
            }
          }

        }
      }
//      if (foundNext)
//        debugArc(current.from(), next.from(), r, "green");
    }
    if (!foundNext) break;

    trajectory.push_back(next.from());
    current = next;
    currentId = nextId;
  }

  trajectory.push_back(current.to());

  return trajectory;
}

std::vector<Point> G::rollBallCavern(std::vector<Point> points, double r) {
//  debugPath(points, "red");
  std::vector<Either> parts;

  // MN is the gate
  Vector MN(points[0], points[points.size() - 1]);
  Point X1 = points[0] + MN.rotate(M_PI / 2) * (r / MN.length());

//  debugCircle(X1, r, "black");
//  debugPoint(X1, "black");
  Point X2 = points[0] + MN.rotate(- M_PI / 2) * (r / MN.length());
  parts.push_back(Either(DirectedArc(X1, X2, points[0], r)));

  for (int i = 0; i < points.size() - 1; i++) {
    Point A = points[i];
    Point B = points[i + 1];
    Vector AB(A, B);
    Point A1 = A + AB.rotate(M_PI / 2) * (r / AB.length());
    Point B1 = B + AB.rotate(M_PI / 2) * (r / AB.length());
    DirectedSegment A1B1(A1, B1);
    parts.push_back(Either(A1B1));
//    debugLine(A1, B1, "black");

    Point B2 = B + AB.rotate(-M_PI / 2) * (r / AB.length());
    DirectedArc B1B2(B1, B2, B, r);
    parts.push_back(Either(B1B2));
//    debugCircle(B, r, "blue");
  }

  DirectedSegment currentSegment = parts[2].directedSegment;
  DirectedArc otherArc = parts[1].directedArc;

  std::vector<Point> trajectory;
  trajectory.push_back(parts[0].from());
  Either current = parts[0];
  int currentId = 0;
  int nextId = 0;
  while (currentId < parts.size()) {
    Either next;
    bool foundNext = false;
    double minDistance = 1e9;

    if (current.isSegment) {
      DirectedSegment currentSegment = current.directedSegment;
      for (int i = currentId + 1; i < parts.size(); i++) {
        Either other = parts[i];
        if (other.isSegment) {
          DirectedSegment otherSegment = other.directedSegment;
          LineSegment ab(currentSegment.from, currentSegment.to),
                      xy(otherSegment.from, otherSegment.to);
          if (G::doIntersect(ab, xy)) {
            foundNext = true;
            Point M;
            if (currentSegment.to == otherSegment.from) {
              M = currentSegment.to;
            } else {
              M = G::intersection(ab, xy);
            }

            if (G::distance(currentSegment.from, M) < minDistance) {
              minDistance = G::distance(currentSegment.from, M);
              next = Either(DirectedSegment(M, otherSegment.to));
              nextId = i;
            }
          }
        } else if (other.isArc) {
          DirectedArc otherArc = other.directedArc;
          Point intersection;
          if (G::is_intersect(currentSegment, otherArc, intersection)) {
            foundNext = true;
            if (G::distance(currentSegment.from, intersection) < minDistance) {
              minDistance = G::distance(currentSegment.from, intersection);
              next = Either(DirectedArc(intersection, otherArc.to, otherArc.center, otherArc.radius));
              nextId = i;
            }
          }
        }
      }

//      if (foundNext)
//        debugLine(current.from(), next.from(), "green");
    } else {
      DirectedArc currentArc = current.directedArc;
      for (int i = currentId + 1; i < parts.size(); i++) {
        Either other = parts[i];
        if (other.isSegment) {
          DirectedSegment otherSegment = other.directedSegment;
          Point intersection;
          if (G::is_intersect(otherSegment, currentArc, intersection)) {
            foundNext = true;
            if (G::distance(currentArc.from, intersection) < minDistance) {
              minDistance = G::distance(currentArc.from, intersection);
              next = Either(DirectedSegment(intersection, otherSegment.to));
              nextId = i;
            }
          }
        } else if (other.isArc) {
          DirectedArc otherArc = other.directedArc;
          Point intersection;
          if (G::is_intersect(currentArc, otherArc, intersection)) {
            foundNext = true;
            if (G::distance(currentArc.from, intersection) < minDistance) {
              minDistance = G::distance(currentArc.from, intersection);
              next = Either(DirectedArc(intersection, otherArc.to, otherArc.center, otherArc.radius));
              nextId = i;
            }
          }

        }
      }
//      if (foundNext)
//        debugArc(current.from(), next.from(), r, "green");
    }
    if (!foundNext) break;

    trajectory.push_back(next.from());
    current = next;
    currentId = nextId;
  }
  Point J = points[points.size() - 1] + MN.rotate(M_PI / 2) * (r / MN.length());
//  debugPoint(J, "black");
//  debugCircle(J, r, "black");
//  debugArc(trajectory[trajectory.size() - 1], J, r, "green");
  trajectory.push_back(J);
//  debugPolygon(trajectory, "red");

//  for (auto p: trajectory) {
//    int random = rand() % 15;
//    if (random <= 1) {
//      debugPoint(p, "black");
//      debugCircle(p, r, "black");
//    }
////    debugPoint(p, "green");
//  }

//  debugCircle(trajectory[0], r, "green");
  return trajectory;
}
std::vector<Point> G::rollBallPolygon(std::vector<Point> points, double r) {

  std::vector<Either> parts;
  for (int i = 0; i < points.size() - 1; i++) {
    Point A = points[i];
    Point B = points[(i + 1) % points.size()];
    Vector AB(A, B);
    Point A1 = A + AB.rotate(M_PI / 2) * (r / AB.length());
    Point B1 = B + AB.rotate(M_PI / 2) * (r / AB.length());
    DirectedSegment A1B1(A1, B1);
    parts.push_back(Either(A1B1));

    Point B2 = B + AB.rotate(-M_PI / 2) * (r / AB.length());
    DirectedArc B1B2(B1, B2, B, r);
    parts.push_back(Either(B1B2));
  }

  DirectedSegment currentSegment = parts[2].directedSegment;
  DirectedArc otherArc = parts[1].directedArc;

  std::vector<Point> trajectory;
  trajectory.push_back(parts[0].from());
  Either current = parts[0];
  int currentId = 0;
  int nextId = 0;
  while (currentId < parts.size()) {
    Either next;
    bool foundNext = false;
    double minDistance = 1e9;

    if (current.isSegment) {
      DirectedSegment currentSegment = current.directedSegment;
      for (int i = currentId + 1; i < parts.size(); i++) {
        Either other = parts[i];
        if (other.isSegment) {
          DirectedSegment otherSegment = other.directedSegment;
          LineSegment ab(currentSegment.from, currentSegment.to),
                      xy(otherSegment.from, otherSegment.to);
          if (G::doIntersect(ab, xy)) {
            foundNext = true;
            Point M;
            if (currentSegment.to == otherSegment.from) {
              M = currentSegment.to;
            } else {
              M = G::intersection(ab, xy);
            }

            if (G::distance(currentSegment.from, M) < minDistance) {
              minDistance = G::distance(currentSegment.from, M);
              next = Either(DirectedSegment(M, otherSegment.to));
              nextId = i;
            }
          }
        } else if (other.isArc) {
          DirectedArc otherArc = other.directedArc;
          Point intersection;
          if (G::is_intersect(currentSegment, otherArc, intersection)) {
            foundNext = true;
            if (G::distance(currentSegment.from, intersection) < minDistance) {
              minDistance = G::distance(currentSegment.from, intersection);
              next = Either(DirectedArc(intersection, otherArc.to, otherArc.center, otherArc.radius));
              nextId = i;
            }
          }
        }
      }
    } else {
      DirectedArc currentArc = current.directedArc;
      for (int i = currentId + 1; i < parts.size(); i++) {
        Either other = parts[i];
        if (other.isSegment) {
          DirectedSegment otherSegment = other.directedSegment;
          Point intersection;
          if (G::is_intersect(otherSegment, currentArc, intersection)) {
            foundNext = true;
            if (G::distance(currentArc.from, intersection) < minDistance) {
              minDistance = G::distance(currentArc.from, intersection);
              next = Either(DirectedSegment(intersection, otherSegment.to));
              nextId = i;
            }
          }
        } else if (other.isArc) {
          DirectedArc otherArc = other.directedArc;
          Point intersection;
          if (G::is_intersect(currentArc, otherArc, intersection)) {
            foundNext = true;
            if (G::distance(currentArc.from, intersection) < minDistance) {
              minDistance = G::distance(currentArc.from, intersection);
              next = Either(DirectedArc(intersection, otherArc.to, otherArc.center, otherArc.radius));
              nextId = i;
            }
          }

        }
      }
    }
    if (!foundNext) break;

    trajectory.push_back(next.from());
    current = next;
    currentId = nextId;
  }

//  for (auto p: trajectory) {
//    int random = rand() % 15;
//    if (random <= 0) {
//      debugPoint(p, "black");
//      debugCircle(p, r, "black");
//    }
////    debugPoint(p, "green");
//  }

//  debugPolygon(trajectory, "green");

//  debugCircle(trajectory[0], r, "purple");

  return trajectory;
}

Point G::closestPointOnSegment(LineSegment ls, Point p) {
  Point lp1 = ls.u, lp2 = ls.v;
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
          ly = ly1 + t * ldy;
//          dx = px - lx,
//          dy = py - ly;

  return Point(lx, ly);
}
Point G::closestPointOnPolygon(std::vector<Point> vs, Point p) {
  double minDis = 1e9;
  Point res;
  for (int i = 0; i < vs.size(); i++) {
    Point A = vs[i];
    Point B = vs[(i + 1) % vs.size()];
    LineSegment AB(A, B);
    Point X = closestPointOnSegment(AB, p);
    if (G::distance(p, X) < minDis) {
      minDis = G::distance(p, X);
      res = X;
    }
  }

  return res;
}
double G::distanceToPolygon(std::vector<Point> vs, Point p) {
  double minDis = 1e9;
  for (int i = 0; i < vs.size(); i++) {
    Point A = vs[i];
    Point B = vs[(i + 1) % vs.size()];
    LineSegment AB(A, B);
    Point X = closestPointOnSegment(AB, p);
    if (G::distance(p, X) < minDis) {
      minDis = G::distance(p, X);
    }
  }

  return minDis;
}
double G::distanceToLineSegment(LineSegment ls, Point p) {
  Point lp1 = ls.u, lp2 = ls.v;
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

double G::deepness(std::vector<Point> cavern) {
  // gate at 0 and size - 1
  LineSegment gate(cavern[0], cavern[cavern.size() - 1]);
  double maxDis = 0;
  for (int i = 0; i < cavern.size(); i++) {
    maxDis = max(maxDis, G::distanceToLineSegment(gate, cavern[i]));
  }


  return maxDis;
}
std::vector<Point> G::flatten(std::vector<Point> path) {
  std::vector<Point> res;
  res.push_back(path[0]);
  for (int i = 1; i < path.size(); i++) {
    if (G::distance(res[res.size() - 1], path[i]) > 20) {
      // Point p(res[res.size() - 1]);
      // while (distance(p, path[i]) > 60){
      //   Vector V(p, path[i]);
      //   p = p + V * (60 / V.length());
      //   res.push_back(p);
      // }
      res.push_back(path[i]);
    } 
    // else {
    //   Point p(midpoint(path[i], res[res.size() - 1]));
    //   // if ((i+1 < path.size()) && (i >= 2))
    //   //   intersection(path[i-2], path[i-1], path[i], path[i+1], p);
    //   res[res.size() - 1] = p;
    // }
  }


  return res;
}

double G::diameter(std::vector<Point> polygon) {
  double maxDis = 0;
  for (int i = 0; i < polygon.size() - 1; i++) {
    for (int j = i + 1; j < polygon.size(); j++) {
      maxDis = max(maxDis, G::distance(polygon[i], polygon[j]));
    }
  }

  return maxDis;
}


tuple<Point, Point> G::hash(std::vector<Point> vs) {
  double xmin = 1e9, ymin = 1e9, xmax = -1, ymax = -1;
  for (auto p: vs) {
    xmin = min(xmin, p.x());
    ymin = min(ymin, p.y());
    xmax = max(xmax, p.x());
    ymax = max(ymax, p.y());
  }

  return make_tuple(Point(xmin, ymin), Point(xmax, ymax));
}

std::vector<Point> G::translate(std::vector<Point> path, double x) {
  if (x == 0) {
    return path;
  } else if (x > 0) {
    return rollBallPath(path, x);
  } else if (x < 0) {
    reverse(path.begin(), path.end());
    auto res = rollBallPath(path, -x);
    reverse(res.begin(), res.end());
    return res;
  }
}

// check whether 3 points (x1, y1), (x2, y2), (x3, y3) are in same line
    bool G::is_in_line(double x1, double y1, double x2, double y2, double x3,
                       double y3) {
        double v1x = x2 - x1;
        double v1y = y2 - y1;
        double v2x = x3 - x1;
        double v2y = y3 - y1;

        if (v1x == 0) {
            if (v2x == 0)
                return true;
            else
                return false;
        }

        if (v1y == 0) {
            if (v2y == 0)
                return true;
            else
                return false;
        }

        if (v2x / v1x == v2y / v1y)
            return true;
        else
            return false;
    }

// check if x is between a and b
    bool G::is_between(double x, double a, double b) {
        return (x - a) * (x - b) < 0 || (x == a && x == b);    // x c (a;b) V x = a = b
        return (x - a) * (x - b) <= 0;                            // x c [a;b] ?
        return (x - a) * (x - b) < 0;                            // x c (a;b) ?
        return (x - a) * (x - b) < 0 || (x == a);                // x c [a;b) ?
        return (x - a) * (x - b) < 0 || (x == b);                // x c (a;b] ?
    }

// check if (x, y) is between (x1, y1) and (x2, y2)
    bool G::is_between(double x, double y, double x1, double y1, double x2,
                       double y2) {
        if (G::is_between(x, x1, x2) && G::is_between(y, y1, y2))
            return true;

        return false;
    }

// angle of line l with vector ox
    Angle G::angle(Line *l) {
        // ax + by + c = 0
        // y = -a/b x - c/b
        if (l->b_ == 0)
            return M_PI / 2;
        return atan(-l->a_ / l->b_);
    }

/**
 * angle of vector (p1, p2) with Ox - "theta" in polar coordinate
 * return angle between (-Pi, Pi]
 */
    Angle G::angle(Point p1, Point p2) {
        return atan2(p2.y_ - p1.y_, p2.x_ - p1.x_);
    }

/**
 * absolute value of angle (p1, p0, p2)
 */
    Angle G::angle(Point *p0, Point *p1, Point *p2) {
        if (*p0 == *p1 || *p0 == *p2)
            return 0;

        double re = fabs(atan2(p1->y_ - p0->y_, p1->x_ - p0->x_) - atan2(p2->y_ - p0->y_, p2->x_ - p0->x_));
        if (re > M_PI)
            re = 2 * M_PI - re;
        return re;
    }

/**
 * angle of vector (p2, p3) to vector (p0, p1)
 */
    Angle G::angle(Point p0, Point p1, Point p2, Point p3) {
        /*
        double re = atan2(p1.y_ - p0.y_, p1.x_ - p0.x_)
                - atan2(p3.y_ - p2.y_, p3.x_ - p2.x_);
                */
        // avoid re = -0.000000 case
        double re1 = atan2(p1.y_ - p0.y_, p1.x_ - p0.x_);
        double re2 = atan2(p3.y_ - p2.y_, p3.x_ - p2.x_);
        if ((p0.x_==p2.x_) && (p0.y_==p2.y_) && (p1.x_==p3.x_) && (p1.y_==p3.y_)) return -1;
        double re = re1 - re2;
        return (re < 0) ? (re + 2 * M_PI) : re;
    }

// angle of vector (v2) to vector (v1)
    Angle G::angle(Vector v1, Vector v2) {
        //double re = atan2(v1.b_, v1.a_) - atan2(v2.b_, v2.a_);
        double re1 = atan2(v1.y, v1.x);
        double re2 = atan2(v2.y, v2.x);
        double re = re1 - re2;
        return (re < 0) ? (re + 2 * M_PI) : re;
    }

// angle from segment ((x2, y2), (x0, y0)) to segment ((x1, y1), (x0, y0))
    double G::angle(double x0, double y0, double x1, double y1, double x2,
                    double y2) {
        //double a = atan2(y1 - y0, x1 - x0) - atan2(y2 - y0, x2 - x0);
        double a1 = atan2(y1 - y0, x1 - x0);
        double a2 = atan2(y2 - y0, x2 - x0);
        double a = a1 - a2;
        return (a < 0) ? (a + 2 * M_PI) : a;
    }

// given p, a, b are collinear. Check if p is in segment ab, p != a, p != b
    bool G::inSegment(Point p, Point a, Point b) {
        bool k = false;
        if (a.x() == b.x()) {
            if (p.y() < std::max(a.y(), b.y()) && p.y() > std::min(a.y(), b.y()))
                k = true;
        } else if (a.y() == b.y()) {
            if (p.x() < std::max(a.x(), b.x()) && p.x() > std::min(a.x(), b.x()))
                k = true;
        } else {
            if (p.x() < std::max(a.x(), b.x()) && p.x() > std::min(a.x(), b.x())
                    && p.y() < std::max(a.y(), b.y()) && p.y() > std::min(a.y(), b.y()))
                k = true;
        }
        return k;
    }

// Check if segment (p1, p2) and segment (p3, p4) are intersected
    bool G::is_intersect2(Point *p1, Point *p2, Point *p3, Point *p4) {
        Line l1 = line(p1, p2);
        Line l2 = line(p3, p4);
        Point in;
        return (intersection(l1, l2, in)
                && (inSegment(in, *p1, *p2))
                && (inSegment(in, *p3, *p4)));
    }

// Check if (x1, y1)(x2, y2) and (x3, y3)(x4, y4) is intersect
    bool G::is_intersect(double x1, double y1, double x2, double y2, double x3,
                         double y3, double x4, double y4) {
        double x, y;
        if (intersection(x1, y1, x2, y2, x3, y3, x4, y4, x, y)) {
            if (G::is_between(x, x1, x2) && G::is_between(x, x3, x4)
                && G::is_between(y, y1, y2) && G::is_between(y, y3, y4))
                return true;
        }
        return false;
    }

    bool G::intersection1(Point p1, Point p2, Point p3, Point p4, Point &p) {
        Line l1 = line(p1, p2);
        Line l2 = line(p3, p4);
        return (intersection(l1, l2, p)
                && (inSegment(p, p1, p2))
                && (inSegment(p, p3, p4)));
    }
// Check if Ellipse e and Line l is intersect
    // int G::intersection(Ellipse *e, Line *l, Point &p1, Point &p2) {
    //     if (l->a_ == 0) {
    //         double y = -l->c_ / l->b_;
    //         double a = e->A();
    //         double b = e->D() + e->B() * y;
    //         double c = e->C() * y * y + e->E() * y + e->F();

    //         int n = quadratic_equation(a, b, c, p1.x_, p2.x_);
    //         if (n)
    //             p1.y_ = p2.y_ = y;
    //         return n;
    //     } else {
    //         double a = e->A() * l->b_ * l->b_ - e->B() * l->a_ * l->b_
    //                    + e->C() * l->a_ * l->a_;
    //         double b = 2 * e->A() * l->b_ * l->c_ - e->B() * l->a_ * l->c_
    //                    - e->D() * l->a_ * l->b_ + e->E() * l->a_ * l->a_;
    //         double c = e->A() * l->c_ * l->c_ - e->D() * l->a_ * l->c_
    //                    + e->F() * l->a_ * l->a_;

    //         int n = quadratic_equation(a, b, c, p1.y_, p2.y_);

    //         if (n) {
    //             p1.x_ = (-l->b_ * p1.y_ - l->c_) / l->a_;
    //             p2.x_ = (-l->b_ * p2.y_ - l->c_) / l->a_;
    //         }

    //         return n;
    //     }
    // }

// intersection point
    bool G::intersection(double x1, double y1, double x2, double y2, double x3,
                         double y3, double x4, double y4, double &x, double &y) {
        double a1, b1, c1, a2, b2, c2;

        G::line(x1, y1, x2, y2, a1, b1, c1);
        G::line(x3, y3, x4, y4, a2, b2, c2);

        return intersection(a1, b2, c1, a2, b2, c2, x, y);
    }

// midpoint of p1 and p2
    Point G::midpoint(Point p1, Point p2) {
        Point re;
        re.x_ = (p1.x_ + p2.x_) / 2;
        re.y_ = (p1.y_ + p2.y_) / 2;
        return re;
    }

    Vector G::vector(Point p1, Point p2) {
        Vector v;
        v.x = p2.x_ - p1.x_;
        v.y = p2.y_ - p1.y_;
        return v;
    }

// vector with slope k
    Vector G::vector(Angle k) {
        k = fmod(k, 2 * M_PI);
        Vector v;
        if (k == M_PI_2) {
            v.x = 0;
            v.y = 1;
        } else if (k == 3 * M_PI_2) {
            v.x = 0;
            v.y = -1;
        } else if (M_PI_2 < k && k < 3 * M_PI_2) {
            v.x = -1;
            v.y = -tan(k);
        } else {
            v.x = 1;
            v.y = tan(k);
        }
        return v;
    }

// line throw p and have slope k
    Line G::line(Point p, Angle k) {
        Line l;
        if (k == M_PI_2 || k == (3 * M_PI_2)) {
            l.a_ = 1;
            l.b_ = 0;
            l.c_ = -p.x_;
        } else {
            l.a_ = -tan(k);
            l.b_ = 1;
            l.c_ = -p.y_ - l.a_ * p.x_;
        }

        return l;
    }

// line contains p and perpendicular with l
    Line G::perpendicular_line(Point *p, Line *l) {
        Line re;
        perpendicular_line(p->x_, p->y_, l->a_, l->b_, l->c_, re.a_, re.b_, re.c_);
        return re;
    }

// line ax + by + c = 0 that pass through (x0, y0) and perpendicular with line created by (x1, y1) and (x2, y2)
    void G::perpendicular_line(double x0, double y0, double x1, double y1,
                               double x2, double y2, double &a, double &b, double &c) {
        double a1, b1, c1;
        G::line(x1, y1, x2, y2, a1, b1, c1);

        perpendicular_line(x0, y0, a1, b1, c1, a, b, c);
    }

// line contains (x0, y0) and perpendicular with line a0x + b0y + c0 = 0
    void G::perpendicular_line(double x0, double y0, double a0, double b0,
                               double c0, double &a, double &b, double &c) {
        a = b0;
        b = -a0;
        c = -a * x0 - b * y0;
    }

// line parallel with l and have distance to l of d
    void G::parallel_line(Line l, double d, Line &l1, Line &l2) {
        Point p;        // choose p is contended by l
        if (l.a_ == 0) {
            p.x_ = 0;
            p.y_ = -l.c_ / l.b_;
        } else {
            p.y_ = 0;
            p.x_ = -l.c_ / l.a_;
        }

        l1.a_ = l2.a_ = l.a_;
        l1.b_ = l2.b_ = l.b_;

        l1.c_ = -d * sqrt(l.a_ * l.a_ + l.b_ * l.b_) - l.a_ * p.x_ - l.b_ * p.y_;
        l2.c_ = d * sqrt(l.a_ * l.a_ + l.b_ * l.b_) - l.a_ * p.x_ - l.b_ * p.y_;
    }

// line contains p and parallel with l
    Line G::parallel_line(Point *p, Line *l) {
        //Line* re = (Line*)malloc(sizeof(Line));
        Line re;
        parallel_line(p->x_, p->y_, l->a_, l->b_, l->c_, re.a_, re.b_, re.c_);
        return re;
    }

// line ax + by +c = 0 that pass through (x0, y0) and parallel with line created by (x1, y1) and (x2, y2)
    void G::parallel_line(double x0, double y0, double x1, double y1, double x2,
                          double y2, double &a, double &b, double &c) {
        double a1, b1, c1;
        line(x1, y1, x2, y2, a1, b1, c1);
        parallel_line(x0, y0, a1, b1, c1, a, b, c);
    }

// line that contains (x0, y0) and parallel with line a0x + b0y + c0 = 0
    void G::parallel_line(double x0, double y0, double a0, double b0, double c0,
                          double &a, double &b, double &c) {
        a = a0;
        b = b0;
        c = -a0 * x0 - b0 * y0;
    }

// the angle bisector line of (p1 p0 p2)
    Line G::angle_bisector(Point p0, Point p1, Point p2) {
        Line re;
        angle_bisector(p0.x_, p0.y_, p1.x_, p1.y_, p2.x_, p2.y_, re.a_, re.b_,
                       re.c_);
        return re;
    }

    void G::angle_bisector(double x0, double y0, double x1, double y1, double x2,
                           double y2, double &a, double &b, double &c) {
        double a1, b1, c1, a2, b2, c2, a3, b3, c3;

        G::line(x0, y0, x1, y1, a1, b1, c1);
        G::line(x0, y0, x2, y2, a2, b2, c2);
        G::line(x1, y1, x2, y2, a3, b3, c3);

        a = a1 / sqrt(a1 * a1 + b1 * b1) - a2 / sqrt(a2 * a2 + b2 * b2);
        b = b1 / sqrt(a1 * a1 + b1 * b1) - b2 / sqrt(a2 * a2 + b2 * b2);
        c = c1 / sqrt(a1 * a1 + b1 * b1) - c2 / sqrt(a2 * a2 + b2 * b2);

        double x, y;

        if (G::intersection(a, b, c, a3, b3, c3, x, y))
            if (G::is_between(x, x1, x2) && G::is_between(y, y1, y2))
                return;

        a = a1 / sqrt(a1 * a1 + b1 * b1) + a2 / sqrt(a2 * a2 + b2 * b2);
        b = b1 / sqrt(a1 * a1 + b1 * b1) + b2 / sqrt(a2 * a2 + b2 * b2);
        c = c1 / sqrt(a1 * a1 + b1 * b1) + c2 / sqrt(a2 * a2 + b2 * b2);
    }

/// tangent lines of ellipse e that have tangent point p
    // Line G::tangent(Ellipse *e, Point *p) {
//  double sin = (e->f2_.y_ - e->f1_.y_) / (2 * e->c());
//  double cos = (e->f2_.x_ - e->f1_.x_) / (2 * e->c());
//
//  Line re;
//  re.a_ = cos * (p->x_ * cos + p->y_ * sin) / (e->a() * e->a()) - sin * (p->x_ * sin + p->y_ * cos) / (e->b() * e->b());
//  re.b_ = sin * (p->x_ * cos + p->y_ * sin) / (e->a() * e->a()) + cos * (p->x_ * sin + p->y_ * cos) / (e->b() * e->b());
//  re.c_ = -1;

    //     Line re;

    //     re.a_ = 2 * e->A() * p->x_ + e->B() * p->y_ + e->D();
    //     re.b_ = 2 * e->C() * p->y_ + e->B() * p->x_ + e->E();
    //     re.c_ = e->D() * p->x_ + e->E() * p->y_ + 2 * e->F();

    //     return re;
    // }

// tangent line of circle c with p
    bool G::tangent(Circle2 *c, Point *p, Line &l1, Line &l2) {
        Point t1;
        Point t2;
        if (tangent_point(c, p, t1, t2)) {
            l1 = line(p, t1);
            l2 = line(p, t2);
            return true;
        } else {
            return false;
        }
    }

// get tangent points of tangent lines of circle that has center O(a, b) and radius r pass through M(x, y)
    bool G::tangent_point(Circle2 *c, Point *p, Point &t1, Point &t2) {
        return tangent_point(c->x_, c->y_, c->radius_, p->x_, p->y_, t1.x_, t1.y_,
                             t2.x_, t2.y_);
    }

// get tangent points of tangent lines of circle that has center O(a, b) and radius r pass through M(x, y)
    bool G::tangent_point(double a, double b, double r, double x, double y,
                          double &t1x, double &t1y, double &t2x, double &t2y) {
        double d = (x - a) * (x - a) + (y - b) * (y - b);

        if (r <= 0 || d < r * r)
            return false;

        t1y = (r * r * (y - b) + r * fabs(x - a) * sqrt(d - r * r)) / d + b;
        t2y = (r * r * (y - b) - r * fabs(x - a) * sqrt(d - r * r)) / d + b;

        if (x - a < 0) {
            t1x = (r * r * (x - a) + r * (y - b) * sqrt(d - r * r)) / d + a;
            t2x = (r * r * (x - a) - r * (y - b) * sqrt(d - r * r)) / d + a;
        } else {
            t1x = (r * r * (x - a) - r * (y - b) * sqrt(d - r * r)) / d + a;
            t2x = (r * r * (x - a) + r * (y - b) * sqrt(d - r * r)) / d + a;
        }

        return true;
    }

    Circle2 G::circumcenter(Point *p1, Point *p2, Point *p3) {
        Circle2 c;
        circumcenter(p1->x_, p1->y_, p2->x_, p2->y_, p3->x_, p3->y_, c.x_, c.y_);
        c.radius_ = distance(c, p1);
        return c;
    }

// find circumcenter (xo, yo)
    void G::circumcenter(double x1, double y1, double x2, double y2, double x3,
                         double y3, double &xo, double &yo) {
        double a1, b1, c1, a2, b2, c2;

        if (!G::is_in_line(x1, y1, x2, y2, x3, y3)) {
            G::perpendicular_bisector(x1, y1, x2, y2, a1, b1, c1);
            G::perpendicular_bisector(x1, y1, x3, y3, a2, b2, c2);
            if (b1 * a2 == b2 * a1) {
                xo = x1;
                yo = y1;
            } else {
                xo = -(c2 * b1 - c1 * b2) / (b1 * a2 - b2 * a1);
                yo = -(c2 * a1 - c1 * a2) / (a1 * b2 - a2 * b1);
            }
        } else {
            xo = x1;
            yo = y1;
        }
    }

// quadratic equation. return number of experiment
    int G::quadratic_equation(double a, double b, double c, double &x1,
                              double &x2) {
        if (a == 0)
            return 0;

        double delta = b * b - 4 * a * c;
        if (delta < 0)
            return 0;

        x1 = (-b + sqrt(delta)) / (2 * a);
        x2 = (-b - sqrt(delta)) / (2 * a);
        return delta > 0 ? 2 : 1;
    }

// find perpendicular bisector of segment ((x1, y1), (x2, y2))
    void G::perpendicular_bisector(double x1, double y1, double x2, double y2,
                                   double &a, double &b, double &c) {
        a = x1 - x2;
        b = y1 - y2;
        c = ((x2) * (x2) + (y2) * (y2) - x1 * x1 - y1 * y1) / 2;
    }

    double G::area(double ax, double ay, double bx, double by, double cx,
                   double cy) {
        double a, b, c;
        a = G::distance(bx, by, cx, cy);
        b = G::distance(cx, cy, ax, ay);
        c = G::distance(ax, ay, bx, by);

        return sqrt((a + b + c) * (b + c - a) * (c + a - b) * (a + b - c) / 16);
    }

/*
 * Extension function
 */

/**
 * directed angle (pb, pa) in clockwise (between [-180;180))
 * */
    Angle G::directedAngle(Point *b, Point *p, Point *a) {
//    if (*a == *p || *a == *b) return 0;
//    double alpha = (atan2(a->y_ - p->y_, a->x_ - p->x_) - atan2(b->y_ - p->y_, b->x_ - p->x_));
//    return alpha;
//    // reduce if |alpha| > PI
//    if (alpha > M_PI){
//        alpha -= M_PI;
//    } else if (alpha < -M_PI){
//        alpha += M_PI;
//    }
//    return alpha;
        double x1, y1, x2, y2;
        x1 = a->x_ - p->x_;
        x2 = b->x_ - p->x_;
        y1 = a->y_ - p->y_;
        y2 = b->y_ - p->y_;
        double dot = x1 * x2 + y1 * y2;      // dot product
        double det = x1 * y2 - y1 * x2;      // determinant
        double angle = atan2(det, dot);  // atan2(y, x) or atan2(sin, cos)

        return angle;
    }

    bool G::lineSegmentIntersection(Point *a, Point *b, Line l,
                                    Point &intersection) {
        intersection.x_ = -1;
        intersection.y_ = -1;
        Line edge = G::line(a, b);

//    if (G::intersection(edge, l, &intersection) && (intersection.x_ >= 0 && intersection.y_ >= 0)) {
        if (G::intersection(edge, l, &intersection)) {
            return onSegment(a, &intersection, b);
//      if((intersection.y_ - a->y_) * (intersection.y_ - b->y_) <= 0)
//            return true;
        }
        return false;
    }

// see http://mathworld.wolfram.com/Circle-CircleIntersection.html
// p1: left side of vector (c1, c2)
// p2: right side of vector (c1, c2)
    int G::circleCircleIntersect(Point c1, double r1, Point c2, double r2,
                                 Point *p1, Point *p2) {
        int nsoln = -1;

        Point c;
        c.x_ = c2.x_ - c1.x_;
        c.y_ = c2.y_ - c1.y_;

        nsoln = G::circleCircleIntersect0a(r1, c, r2, p1, p2);
        /* Translate back. */
        p1->x_ += c1.x_;
        p1->y_ += c1.y_;
        p2->x_ += c1.x_;
        p2->y_ += c1.y_;
        return nsoln;
    }

/*---------------------------------------------------------------------
 circleCircleIntersect0a assumes that the first circle is centered on the origin.
 Returns # of intersections: 0, 1, 2, 3 (inf); point in p.
 ---------------------------------------------------------------------*/
    int G::circleCircleIntersect0a(double r1, Point c2, double r2, Point *p1,
                                   Point *p2) {
        double dc2; /* dist to center 2 squared */
        double rplus2, rminus2; /* (r1 +/- r2)^2 */
        double f; /* fraction along c2 for nsoln=1 */

        /* Handle special cases. */
        dc2 = c2.x_ * c2.x_ + c2.y_ * c2.y_;
        rplus2 = (r1 + r2) * (r1 + r2);
        rminus2 = (r1 - r2) * (r1 - r2);

        /* No solution if c2 out of reach + or -. */
        if ((dc2 > rplus2) || (dc2 < rminus2))
            return 0;

        /* One solution if c2 just reached. */
        /* Then solution is r1-of-the-way (f) to c2. */
        if (dc2 == rplus2) {
            f = r1 / (double) (r1 + r2);
            p1->x_ = p2->x_ = f * c2.x_;
            p1->y_ = p2->y_ = f * c2.y_;
            return 1;
        }
        if (dc2 == rminus2) {
            if (rminus2 == 0) { /* Circles coincide. */
                p1->x_ = p2->x_ = r1;
                p1->y_ = p2->y_ = 0;
                return 3; // infinite
            }
            f = r1 / (double) (r1 - r2);
            p1->x_ = p2->x_ = f * c2.x_;
            p1->y_ = p2->y_ = f * c2.y_;
            return 1;
        }

        /* Two intersections. */
        return G::circleCircleIntersect0b(r1, c2, r2, p1, p2);
    }

/*---------------------------------------------------------------------
 circleCircleIntersect0b also assumes that the 1st circle is origin-centered.
 ---------------------------------------------------------------------*/
    int G::circleCircleIntersect0b(double r1, Point c2, double r2, Point *p1,
                                   Point *p2) {
        double a2; /* center of 2nd circle when rotated to x-axis */
        Point q1, q2; /* solution when c2 on x-axis */
        double cost, sint; /* sine and cosine of angle of c2 */

        /* Rotate c2 to a2 on x-axis. */
        a2 = sqrt(c2.x_ * c2.x_ + c2.y_ * c2.y_);
        cost = c2.x_ / a2;
        sint = c2.y_ / a2;

        G::circleCircleIntersect00(r1, a2, r2, &q1, &q2);

        /* Rotate back */
        p1->x_ = cost * q1.x_ + -sint * q1.y_;
        p1->y_ = sint * q1.x_ + cost * q1.y_;
        p2->x_ = cost * q2.x_ + -sint * q2.y_;
        p2->y_ = sint * q2.x_ + cost * q2.y_;

        return 2;
    }

/*---------------------------------------------------------------------
 circleCircleIntersect00 assumes circle centers are (0,0) and (a2,0).
 ---------------------------------------------------------------------*/
    void G::circleCircleIntersect00(double r1, double a2, double r2, Point *p1,
                                    Point *p2) {
        double r1sq, r2sq;
        r1sq = r1 * r1;
        r2sq = r2 * r2;

        /* Return only positive-y soln in p. */
        p1->x_ = p2->x_ = (a2 + (r1sq - r2sq) / a2) / 2;
        p1->y_ = sqrt(r1sq - p1->x_ * p1->x_);
        p2->y_ = -p1->y_;
    }

// see at http://mathworld.wolfram.com/Circle-LineIntersection.html
// return intersection of (c,r) with line (i1, i2)
    int G::circleLineIntersect(Point c, double r, Point i1, Point i2, Point *p1,
                               Point *p2) {
        i1.x_ = i1.x_ - c.x_;
        i2.x_ = i2.x_ - c.x_;
        i1.y_ = i1.y_ - c.y_;
        i2.y_ = i2.y_ - c.y_;

        int re = circleLineIntersect00(r, i1, i2, p1, p2);
        p1->x_ += c.x_;
        p2->x_ += c.x_;
        p1->y_ += c.y_;
        p2->y_ += c.y_;
        return re;
    }

// intersect of line l and circle ((0,0), r)
    int G::circleLineIntersect00(double r, Point i1, Point i2, Point *p1,
                                 Point *p2) {
        double dx = i2.x_ - i1.x_;
        double dy = i2.y_ - i1.y_;
        double dr = sqrt(dx * dx + dy * dy);
        double d = i1.x_ * i2.y_ - i1.y_ * i2.x_;
        double delta = r * r * dr * dr - d * d;

        if (delta < 0) {
            return 0;
        } else if (delta == 0) {
            p1->x_ = p2->x_ = d * dy / (dr * dr);
            p1->y_ = p2->y_ = -d * dx / (dr * dr);
            return 1;
        } else {
            int sgn = dy < 0 ? -1 : 1;
            p1->x_ = (d * dy + sgn * dx * sqrt(delta)) / (dr * dr);
            p2->x_ = (d * dy - sgn * dx * sqrt(delta)) / (dr * dr);
            p1->y_ = (-d * dx + fabs(dy) * sqrt(delta)) / (dr * dr);
            p2->y_ = (-d * dx - fabs(dy) * sqrt(delta)) / (dr * dr);
            return 2;
        }
    }

// clockwise directed angle between vector (pa, px // Ox) in range [-180, 180]
    Angle G::angle_x_axis(Point *a, Point *p) {
        double x1, y1;
        x1 = a->x_ - p->x_;
        y1 = a->y_ - p->y_;

        double a1 = atan2(y1, x1);
        return a1;
    }

// Given three colinear points p, q, r, the function checks if
// point q lies on line segment 'pr'
    bool G::onSegment(Point p, Point q, Point r) {
//  return q.x_ <= g_max(p.x_, r.x_) && q.x_ >= g_min(p.x_, r.x_) &&
//    q.y_ <= g_max(p.y_, r.y_) && q.y_ >= g_min(p.y_, r.y_);
        return ((q.x_ < p.x_) != (q.x_ < r.x_)) || ((q.y_ < p.y_) != (q.y_ < r.y_));
    }

// To find orientation of ordered triplet (a, b, c).
// The function returns following values
// 0 --> collinear
// 1 --> Clockwise
// 2 --> Counterclockwise
    int G::orientation2(Point a, Point b, Point c) {
        /* old implement based on http://www.dcs.gla.ac.uk/~pat/52233/slides/Geometry1x1.pdf - page 10
         double val = (b.y_ - a.y_) * (c.x_ - b.x_) - (b.x_ - a.x_) * (c.y_ - b.y_);
         if (val == 0) return 0; // colinear
         return (val > 0)? 1: 2; // clock or counterclock wise
         */

        // https://www.cs.princeton.edu/~rs/AlgsDS07/16Geometric.pdf - page 9
        // double area = (b.x_ - a.x_) * (c.y_ - a.y_) - (b.y_ - a.y_) * (c.x_ - a.x_);
        double area1 = (b.x_ - a.x_) * (c.y_ - a.y_);
        double area2 = (b.y_ - a.y_) * (c.x_ - a.x_);
        double area = area1 - area2;
        return area == 0 ? 0 : (area > 0 ? 2 : 1);
    }

// The main function that returns true if line segment 'p1q1'
// and 'p2q2' intersect.
    bool G::doIntersect(Point p1, Point q1, Point p2, Point q2) {
        // Find the four orientations needed for general and
        // special cases
        int o1 = orientation2(p1, q1, p2);
        int o2 = orientation2(p1, q1, q2);
        int o3 = orientation2(p2, q2, p1);
        int o4 = orientation2(p2, q2, q1);

        // General case
        if (o1 != o2 && o3 != o4)
            return true;

        // Special Cases
        // p1, q1 and p2 are colinear and p2 lies on segment p1q1
        if (o1 == 0 && onSegment(p1, p2, q1))
            return true;

        // p1, q1 and p2 are colinear and q2 lies on segment p1q1
        if (o2 == 0 && onSegment(p1, q2, q1))
            return true;

        // p2, q2 and p1 are colinear and p1 lies on segment p2q2
        if (o3 == 0 && onSegment(p2, p1, q2))
            return true;

        // p2, q2 and q1 are colinear and q1 lies on segment p2q2
        if (o4 == 0 && onSegment(p2, q1, q2))
            return true;

        return false; // Doesn't fall in any of the above cases
    }

/**
 * Check if point is inside P1P2P3 triangle
 */
    bool G::isPointLiesInTriangle(Point *p, Point *p1, Point *p2, Point *p3) {
        // barycentric algorithm
        double alpha = ((p2->y_ - p3->y_) * (p->x_ - p3->x_)
                        + (p3->x_ - p2->x_) * (p->y_ - p3->y_))
                       / ((p2->y_ - p3->y_) * (p1->x_ - p3->x_)
                          + (p3->x_ - p2->x_) * (p1->y_ - p3->y_));
        double beta = ((p3->y_ - p1->y_) * (p->x_ - p3->x_)
                       + (p1->x_ - p3->x_) * (p->y_ - p3->y_))
                      / ((p2->y_ - p3->y_) * (p1->x_ - p3->x_)
                         + (p3->x_ - p2->x_) * (p1->y_ - p3->y_));
        double gamma = 1.0f - alpha - beta;

        return gamma >= 0 && alpha >= 0 && beta >= 0;
    }

// get aggregation between segment [a1, a2] and [b1, b2]
// return to a1, a2
    int G::segmentAggregation(Point *a1, Point *a2, Point *b1, Point *b2) {
        Point tmp;
        if (a1->x_ > a2->x_ || (a1->x_ == a2->y_ && a1->y_ > a2->y_)) {
            tmp = *a1;
            *a1 = *a2;
            *a2 = tmp;
        }

        if (b1->x_ > b2->x_ || (b1->x_ == b2->y_ && b1->y_ > b2->y_)) {
            tmp = *b1;
            *b1 = *b2;
            *b2 = tmp;
        }

        if (onSegment(a1, b1, a2)) {
            *a1 = *b1;
            if (onSegment(a1, b2, a2)) {
                *a2 = *b2;
            }
            return 1;
        } else {
            if (onSegment(a1, b2, a2)) {
                *a2 = *b2;
                return 1;
            } else {
                if (onSegment(b1, a1, b2)) {
                    return 1;
                }
            }
        }

        return 0;
    }

    int G::orientation(Point *p, Point q, Point *r) {
        Point p1, r1;
        p1.x_ = p->x_;
        p1.y_ = p->y_;
        r1.x_ = r->x_;
        r1.y_ = r->y_;
        return orientation2(p1, q, r1);
    }

    Point G::intersectSections(Point &begin1, Point &end1, Point &begin2,
                               Point &end2) {
        double x1 = begin1.x();
        double y1 = begin1.y();
        double x2 = end1.x();
        double y2 = end1.y();
        double x3 = begin2.x();
        double y3 = begin2.y();
        double x4 = end2.x();
        double y4 = end2.y();
        double a = determinant(x1, y1, x2, y2);
        double b = determinant(x3, y3, x4, y4);
        double c = determinant(x1 - x2, y1 - y2, x3 - x4, y3 - y4);
        double x = determinant(a, x1 - x2, b, x3 - x4) / c;
        double y = determinant(a, y1 - y2, b, y3 - y4) / c;
        if (x1 < x && x < x2 && x3 < x && x < x4 && y1 < y && y < y2 && y3 < y
            && y < y4) {
            return Point(x, y);
        } else {
            return Point(NaN, NaN);
        }

    }

    double G::determinant(double a1, double a2, double b1, double b2) {
        return a1 * b2 - a2 * b1;
    }

    double G::getVectorAngle(Point p1, Point p2) {
        Vector v = vector(p1, p2);
        double angle = atan2(-v.y, v.x);
        if (angle < 0)
            angle += 2 * M_PI;
        return angle;
    }

// check if a Point is inside (including lies on edge) of a grid
    bool G::isPointInsidePolygon(Point p, std::vector<Point> polygon) {
        bool odd = false;
        for (unsigned int i = 0; i < polygon.size(); i++) {
            Point tmp = polygon[i];
            Point tmp_next = polygon[(i + 1) % polygon.size()];
            if (G::is_in_line(tmp, tmp_next, p)) {
                if (G::onSegment(tmp, p, tmp_next)) {
                    return true;
                }
            }
            if ((tmp.y_ > p.y_) != (tmp_next.y_ > p.y_) &&
                (p.x_ < (tmp_next.x_ - tmp.x_) * (p.y_ - tmp.y_) / (tmp_next.y_ - tmp.y_) + tmp.x_))
                odd = !odd;
        }
        return odd;
    }

    bool G::isPointReallyInsidePolygon(Point p, std::vector<Point> polygon) {
        bool odd = false;

        for (unsigned int i = 0; i < polygon.size(); i++) {
            Point tmp = polygon[i];
            Point tmp_next = polygon[(i + 1) % polygon.size()];
            if (G::is_in_line(tmp, tmp_next, p)) {
                if (G::onSegment(tmp, p, tmp_next)) {
                    return false;
                }
            }
            if ((tmp.y_ > p.y_) != (tmp_next.y_ > p.y_) &&
                (p.x_ < (tmp_next.x_ - tmp.x_) * (p.y_ - tmp.y_) / (tmp_next.y_ - tmp.y_) + tmp.x_))
                odd = !odd;
        }
        return odd;
    }

    bool G::segmentPolygonIntersect(Point s, Point d, std::vector<Point> p) {
        int num_intersection = 0;
        for (unsigned int i = 0; i < p.size(); i++) {
            int j = (i == p.size() - 1 ? 0 : (i + 1));
            if (G::is_intersect2(s, d, p[i], p[j]))
                num_intersection++;
        }
        return num_intersection > 0;
    }

    bool G::segmentPolygonIntersect(Point s, Point d, std::vector<Point> v, Point &p) {
        for (unsigned int i = 0; i < v.size() - 1; i++) {
            if (G::intersection1(s, d, v[i], v[i+1], p)) {
                return true;
            }
        }
    }
    bool G::isSegmentInsidePolygon(Point s, Point d, std::vector<Point> p) {
        if (G::segmentPolygonIntersect(s, d, p))
            return false;
        else {
            if (!G::isPointReallyInsidePolygon(G::midpoint(s, d), p)) // todo: cung chua chuan lam nhung tam on trong hau het cac truong hop can dung
                return false;                                   // todo: cung co the midpoint van la dinh cua polygon
            else return true;
        }
    }

    // do not consider gate (p[0]p[p.size()-1])
    bool G::segmentPolygonIntersect1(Point s, Point d, std::vector<Point> p) {
        int num_intersection = 0;
        for (unsigned int i = 0; i < p.size() - 1; i++) {
            if (G::is_intersect2(s, d, p[i], p[i + 1 ])) num_intersection++;
        }
        return num_intersection > 0;
    }

    bool G::isSegmentInsidePolygon1(Point s, Point d, std::vector<Point> p) {
        if (G::segmentPolygonIntersect1(s, d, p))
            return false;
        else {
            if (!G::isPointInsidePolygon(G::midpoint(s, d), p)) // todo: cung chua chuan lam nhung tam on trong hau het cac truong hop can dung
                return false;                                   // todo: cung co the midpoint van la dinh cua polygon
            else return true;
        }
    }

    bool G::isSameSign(double x, double y) {
        return (x >= 0) ^ (y < 0);
    }

    int G::isMonotoneLine(int a, int b, std::vector<Point> hole) {
      int n = hole.size();
      if ((b-a+n)%n <= 1) return 1;
      if ((a-b+n)%n <= 1) return -1;
      int start, end;
      if ((b-a+n)%n <= n/2){
      // if (true){
      // if (a < b) {
        start = a;
        end = b;
      }
      else {
        start = b;
        end = a;
      }
      // trace() << "start " << start;

      Point A = hole[start];
      Point B = hole[end];
      Point M_0 = hole[(start+1)%n];
      Vector AB(A, B);
      Vector AM_0(A, M_0);
      double c_0 = AB.crossProduct(AM_0);
      for (int i=(start+2)%n; i!=end; i=(i+1)%n){
        Point M_i = hole[i];
        Vector AM_i(A, M_i);
        double c_i = AB.crossProduct(AM_i);
        if (!isSameSign(c_0, c_i)) return 0;
      }
      // trace() << "side " << (c_0 > 0) - (c_0 < 0);
      return (start == a) - (start == b);
      // return (c_0 > 0) - (c_0 < 0);
    }

    int G::isMonotoneLine(Point A, Point B, std::vector<Point> hole) {
      int a = getVertexIndexInPolygon(A, hole);
      int b = getVertexIndexInPolygon(B, hole);
      if ((a == -1) || (b == -1)) return 0;
      return isMonotoneLine(a, b, hole);
    }

    int G::getVertexIndexInPolygon(Point P, std::vector<Point> hole){
      int index = -1;
      for (int i=0; i<hole.size(); i++){
        if (P == hole[i]){
          index = i;
          break;
        }
      }
      return index;
    }

    bool G::is_intersect(DirectedArc arc, std::vector<Point> hole){
      for (int i=0; i<hole.size(); i++){
        DirectedSegment seg(hole[i], hole[(i+1)%hole.size()]);
        Point intersection;
        if (is_intersect(seg, arc, intersection)) return true;
      }
      return false;
    }

    bool G::is_intersect(LineSegment seg, std::vector<Point> hole){
      for (int i=0; i<hole.size(); i++){
        LineSegment seg_i(hole[i], hole[(i+1)%hole.size()]);
        if (doIntersect(seg, seg_i)) return true;
      }
      return false;
    }

    Point G::randomPointInPolygon(std::vector<Point> vs){
      double minx = DBL_MAX, miny = DBL_MAX, maxx = - DBL_MAX, maxy = - DBL_MAX;
      for (Point p : vs){
        if (p.x_ < minx) minx = p.x_;
        if (p.y_ < miny) miny = p.y_;
        if (p.x_ > maxx) maxx = p.x_;
        if (p.y_ > maxy) maxy = p.y_;
      }
      // trace() << DBL_MAX;
      // trace() << "minx " << minx;
      // trace() << "maxx " << maxx;
      // trace() << "miny " << miny;
      // trace() << "maxy " << maxy;

      int count = 0;
      while (true)
      { 
        count++;
        double fx = (double)rand() / RAND_MAX;
        double x = minx + fx * (maxx - minx);
        double fy = (double)rand() / RAND_MAX;
        double y = miny + fy * (maxy - miny);
        Point p(x,y);
        if (G::pointInPolygon(vs, p)){
          // trace() << "fx " << fx;
          // trace() << "fy " << fy;
          // trace() << "count " << count;
          return p;
        }
      }
      
    }

