#include "geometry.h"

Point2D getMidPoint(Point2D &entryPoint, Point2D &exitPoint)
{

    // return {0.5 * (entryPoint[0] + exitPoint[0]), 0.5 * (entryPoint[1] + exitPoint[1])};
    return (entryPoint + exitPoint) * 0.5;
}

double calculateLength(std::array<double, 2> &exitPoint, std::array<double, 2> &entryPoint)
{
    return std::sqrt(std::pow(exitPoint[0] - entryPoint[0], 2) + std::pow(exitPoint[1] - entryPoint[1], 2));
}

// 返回结果：t和dist
// t为投影至线段上的比例，dist为点P到其投影点（垂足）的距离
// 如果t<0：投影点在A外侧
// 如果t>0：投影点在B外侧
// 如果0<t<1：投影点在线段AB上
std::pair<double, double> calculateDistToPolyline(Point2D &ptP, Point2D &ptA, Point2D &ptB)
{
    Point2D AB = ptB - ptA;
    Point2D AP = ptP - ptA;
    double ab2 = dot(AB, AB);

    double segmentLen = calculateLength(ptA, ptB);

    if (segmentLen == 0)
    {
        return {0, length(ptP - ptA)};
    }

    // 计算投影比例 t
    // t = [(P-A) . (B-A)] / |B-A|^2
    double t = dot(AP, AB) / ab2;
    Point2D ptQ = ptA + AB * t;
    return {t, length(ptP - ptQ)};
}

// 圆内任意两个弦AB,CD相交于点O,则
// AO*BO = CO *DO
// (2*R-chordToCircum)*chordToCircum = chord/2 * chord/2
double getRadiusByChordCircum(double chord, double chordToCircum)
{
    double radius = (chord * chord * 0.25 + chordToCircum * chordToCircum) / (2 * chordToCircum);
    return radius;
}

std::array<double, 2> getCenter(std::array<double, 2> chordMid, double angle, double radius, double chordtoCircum)
{

    // 判断angle大小
    // 如果angle大于pi/2, 则取angle-pi/2
    // 如果angle小于pi/2, 则取angle+pi/2
    if (angle > HALF_PI)
    {
        angle = angle - HALF_PI;
    }
    else
    {
        angle = angle + HALF_PI;
    }

    std::array<double, 2> center;
    double centertoChord = radius - chordtoCircum;
    double radiusX = chordMid[0] + centertoChord * std::cos(angle);
    double radiusY = chordMid[1] + centertoChord * std::sin(angle);
    center[0] = radiusX;
    center[1] = radiusY;
    return center;
}

// 根据指定的起始点和终止点截取折线段, 需要以指定的起始点开始, 指定的终止点结束
// 实现过程:
// 找到start_index和end_index
// start_index相应的元素其x值要小于start_x, end_index相应的元素其x值要大于end_x
// 这样根据左右两边的index截取出来vector
// 再将start_point和end_point替换新vector的第一个元素和最后一个元素
MultiPoints2D cutFromPolyLine(const MultiPoints2D &polyline, const Point2D &startPoint, const Point2D &endPoint)
{

    MultiPoints2D result;
    Point2D leftPt = startPoint;
    Point2D rightPt = endPoint;

    int startIdx = 0;
    int endIdx = polyline.size() - 1;

    if (leftPt[0] > rightPt[0])
    {
        std::swap(leftPt, rightPt);
    }
    // TODO: bug fix
    if (leftPt[0] < polyline[0][0])
    {
        leftPt = polyline[0];
    }
    if (rightPt[0] > polyline[polyline.size() - 1][0])
    {
        rightPt = polyline[polyline.size() - 1];
    }
    double startX = leftPt[0];
    double endX = rightPt[0];

    if (startX == endX)
    {
        throw std::runtime_error("the startPoint seems same as the endPoint, please check them");
    }
    else
    {

        while (startIdx < polyline.size() - 1)
        {
            if ((polyline[startIdx][0] <= startX) && ((polyline[startIdx + 1][0] > startX)))
            {
                break;
            }
            else
            {
                startIdx++;
            }
        }
        while (endIdx > 0)
        {
            if ((polyline[endIdx - 1][0] < endX) && (polyline[endIdx][0] >= endX))
            {
                break;
            }
            else
            {
                endIdx--;
            }
        }
    }
    if (endIdx <= startIdx)
    {
        throw std::runtime_error("the startPoint and endPoint are not in the poly_line,please check them");
    }
    else
    {
        // 按照起止索引遍历poly_line中的元素, 并将其加入cutted_poly_line
        auto startInterator = polyline.begin() + startIdx;
        auto endInterator = polyline.begin() + endIdx + 1;
        result.insert(result.end(), startInterator, endInterator);
        result.front() = leftPt;
        result.back() = rightPt;
    }

    return result;
}

std::vector<double> interpolate(double start, double end, int numPoints)
{
    std::vector<double> result;
    double minVal = std::min(start, end);
    double maxVal = std::max(start, end);

    double dx = (maxVal - minVal) / (numPoints - 1);
    for (int i = 0; i < numPoints; i++)
    {
        double x = minVal + i * dx;
        result.push_back(x);
    }
    return result;
}

// numPoints 为插值后点的数量(包括原来的polyline起止点)，因此numPoints >= 3
MultiPoints2D interpolatedPolyLine(const MultiPoints2D &polyline, int numPoints)
{
    if (numPoints < 2)
    {
        throw std::runtime_error("interpolation number must greater than 2");
    }
    MultiPoints2D result;
    result.reserve(numPoints);
    if (polyline.size() < 2)
    {
        throw std::runtime_error("polyline must have at least two points.");
    }
    else if (polyline.size() == 2)
    {
        double dx = (polyline[1][0] - polyline[0][0]) / (numPoints - 1);
        double dy = (polyline[1][1] - polyline[0][1]) / (numPoints - 1);
        for (int i = 0; i < numPoints; i++)
        {
            double x = polyline[0][0] + i * dx;
            double y = polyline[0][1] + i * dy;
            result.push_back({x, y});
        }
    }
    else
    {
        double dx = (polyline.back()[0] - polyline.front()[0]) / (numPoints - 1);

        for (int i = 0; i < numPoints; i++)
        {
            double x = polyline.front()[0] + i * dx;
            double y = getYbyX(polyline, x);
            if (std::isnan(y))
            {
                throw std::runtime_error("the polyline is not monotonic, please check it");
            }
            else
            {
                result.push_back({x, y});
            }
        }
    }
    return result;
}

double getYbyX(const MultiPoints2D &polyline, double x)
{
    double result = std::nan(""); // 初始化为NaN
    int plineSize = polyline.size();

    if (plineSize < 2)
    {
        throw std::runtime_error("polyline must have at least two points.");
    }
    else
    {
        for (int i = 0; i < plineSize - 1; i++)
        {
            if (x >= polyline[i][0] && x <= polyline[i + 1][0])
            {
                // 线性插值
                double slope = (polyline[i + 1][1] - polyline[i][1]) / (polyline[i + 1][0] - polyline[i][0]);
                result = polyline[i][1] + slope * (x - polyline[i][0]);
                break;
            }
        }
    }

    return result;
}

// 实现原理:
//  vect_delta = p1 -center
//  vect_dir = p2 - p1
//  p1p2上的任意一点p到圆心的向量可表示为: vect_p = t*vect_dir + vect_delta
//  则p1p2与圆的交点满足: (vect_p-center)^2 = R^2
//  问题就转换成求解关于t的二元一次方程
int intersectionNum(const MultiPoints2D &polyline,
                    const std::array<double, 2> &center,
                    double radius)
{
    int num = 0;
    int plineSize = polyline.size();

    if (plineSize < 2)
    {
        throw std::runtime_error("polyline must have at least two points.");
    }
    else
    {
        for (int i = 0; i < plineSize - 1; i++)
        {
            std::array<double, 2> p1 = polyline[i];
            std::array<double, 2> p2 = polyline[i + 1];
            std::array<double, 2> dir = {
                p2[0] - p1[0],
                p2[1] - p1[1]};

            // p = p1 + t * dir
            // ||p - center||^2 = R^2
            std::array<double, 2> delta = {
                p1[0] - center[0],
                p1[1] - center[1]};

            // a = |dir|^2
            // b = 2 * (dir * delta)
            // c = delta * delta - R^2
            double a = dir[0] * dir[0] + dir[1] * dir[1];
            double b = 2 * (dir[0] * delta[0] + dir[1] * delta[1]);
            double c = delta[0] * delta[0] + delta[1] * delta[1] - radius * radius;

            // 解二次方程 a*t² + b*t + c = 0
            double discriminant = b * b - 4 * a * c;

            std::vector<double> t_values;

            if (discriminant > EPSILON)
            {
                double t1 = (-b + std::sqrt(discriminant)) / (2 * a);
                double t2 = (-b - std::sqrt(discriminant)) / (2 * a);
                t_values.push_back(t1);
                t_values.push_back(t2);
            }
            else if (abs(discriminant) <= EPSILON)
            {
                double t = -b / (2 * a);
                t_values.push_back(t);
            }
            else
            {
                continue;
            }
            auto unique_end = std::unique(t_values.begin(), t_values.end(), [&](double x, double y)
                                          { return std::abs(x - y) < EPSILON; });
            t_values.erase(unique_end, t_values.end());
            for (double t : t_values)
            {
                // Only count intersections strictly inside the segment
                if (t > EPSILON && t < 1.0 - EPSILON)
                {
                    num++;
                }
                // 只记t=0, 不计t=1,除非是多线段的终点
                // t near 0 or 1 → boundary case
                else if (t >= -EPSILON && t <= EPSILON)
                {
                    // avoid double count: count only if i==0
                    num++;
                }
                else if (t >= 1.0 - EPSILON && t <= 1.0 + EPSILON)
                {
                    // avoid double count: count only if it's NOT the start of next segment
                    // Next segment start index is i+1, so only count here if this is last segment
                    if (i == plineSize - 2)
                        num++;
                }
            }
        }
        return num;
    }
}

std::optional<Point2D> segmentIntersection(const Point2D &A, const Point2D &B, const Point2D &C, const Point2D &D)
{
    Point2D r = B - A;
    Point2D s = D - C;

    double denom = cross(r, s);
    // 平行或共线
    if (std::abs(denom) < EPSILON)
        return std::nullopt;

    double t = cross(C - A, s) / denom;
    double u = cross(C - A, r) / denom;

    // 相交点,在线段内
    if (t >= 0 && t <= 1 && u >= 0 && u <= 1)
    {
        return A + r * t;
    }
    return std::nullopt;
}

// 实现原理:
//  vect_delta = p1 -center
//  vect_dir = p2 - p1
//  p1p2上的任意一点p到圆心的向量可表示为: vect_p = t*vect_dir + vect_delta
//  则p1p2与圆的交点满足: (vect_p-center)^2 = R^2
//  问题就转换成求解关于t的二元一次方程
MultiPoints2D polyLineArcIntersection(const MultiPoints2D &polyline,
                                      const Point2D &center,
                                      double radius)
{
    MultiPoints2D result;
    int plineSize = polyline.size();
    auto pointEqual = [&](const Point2D &a, const Point2D &b)
    {
        return std::abs(a[0] - b[0]) < EPSILON &&
               std::abs(a[1] - b[1]) < EPSILON;
    };
    auto angle = [&](const Point2D &p)
    {
        return std::atan2(p[1] - center[1], p[0] - center[0]);
    };

    if (plineSize < 2)
    {
        throw std::runtime_error("polyline must have at least two points.");
    }
    else
    {
        for (int i = 0; i < plineSize - 1; i++)
        {
            Point2D p1 = polyline[i];
            Point2D p2 = polyline[i + 1];

            Point2D dir = p2 - p1;

            // p = p1 + t * dir
            // ||p - center||^2 = R^2
            Point2D delta = p1 - center;

            // a = |dir|^2
            // b = 2 * (dir * delta)
            // c = delta * delta - R^2
            // double a = dir[0] * dir[0] + dir[1] * dir[1];
            double a = dot(dir, dir);
            double b = 2 * dot(dir, delta);
            double c = dot(delta, delta) - radius * radius;

            // 解二次方程 a*t² + b*t + c = 0
            double discriminant = b * b - 4 * a * c;

            std::vector<double> t_values;

            if (discriminant > EPSILON)
            {
                double t1 = (-b + std::sqrt(discriminant)) / (2 * a);
                double t2 = (-b - std::sqrt(discriminant)) / (2 * a);
                t_values.push_back(t1);
                t_values.push_back(t2);
            }
            else if (abs(discriminant) <= EPSILON)
            {
                double t = -b / (2 * a);
                t_values.push_back(t);
            }
            else
            {
                continue;
            }

            for (double t : t_values)
            {
                if (t < -EPSILON || t > 1.0 + EPSILON)
                    continue;
                // clamp
                t = std::max(0.0, std::min(1.0, t));

                Point2D interpPt = p1 + dir * t;

                // 去重
                bool duplicate = false;
                for (const auto &p : result)
                {
                    if (pointEqual(p, interpPt))
                    {
                        duplicate = true;
                        break;
                    }
                }
                if (!duplicate)
                {
                    result.push_back(interpPt);
                }
            }
        }
        // 按 x（第一列）排序
        std::sort(result.begin(), result.end(), [&](const Point2D &a, const Point2D &b)
                  {
                      if (std::abs(a[0] - b[0]) > EPSILON)
                          return a[0] < b[0];
                      return a[1] < b[1]; });
        // // 按照圆弧顺序排序
        // std::sort(result.begin(), result.end(),
        //           [&](const Point2D &a, const Point2D &b)
        //           {
        //               return angle(a) < angle(b);
        //           });

        return result;
    }
}

MultiPoints2D twoPolyLineIntersection(const MultiPoints2D &pLine1, const MultiPoints2D &pLine2)
{
    MultiPoints2D result;

    int pL1Size = pLine1.size();
    int pL2Size = pLine2.size();
    // 检查是否重复
    auto pointEqual = [&](const Point2D &a, const Point2D &b)
    {
        return std::abs(a[0] - b[0]) < EPSILON &&
               std::abs(a[1] - b[1]) < EPSILON;
    };
    for (int p1Idx = 0; p1Idx < pL1Size - 1; p1Idx++)
    {
        for (int p2Idx = 0; p2Idx < pL2Size - 1; p2Idx++)
        {
            auto intersectonPt = segmentIntersection(pLine1[p1Idx], pLine1[p1Idx + 1], pLine2[p2Idx], pLine2[p2Idx + 1]);
            if (intersectonPt)
            {
                // 去重
                bool duplicate = false;
                for (const auto &p : result)
                {
                    if (pointEqual(p, intersectonPt.value()))
                    {
                        duplicate = true;
                        break;
                    }
                }
                if (!duplicate)
                    result.push_back(intersectonPt.value());
            }
        }
    }
    // 按 x（第一列）排序
    std::sort(result.begin(), result.end(),
              [&](const Point2D &a, const Point2D &b)
              {
                  if (std::abs(a[0] - b[0]) > EPSILON)
                      return a[0] < b[0];
                  return a[1] < b[1];
              });

    return result;
}

// 从两个点和圆心、圆弧去生成CAD多线段表示法
PolyLineCAD toPolyLineCAD(const Point2D &A, const Point2D &B, const Point2D &center, double radius, bool ccw)
{

    if (radius < EPSILON)
        throw std::runtime_error("Invalid radius");

    // 声明结果
    PolyLineCAD result;
    result.reserve(2);
    double bulge;

    Point2D CA = A - center;
    Point2D CB = B - center;

    if (std::abs(length(CA) - radius) > EPSILON || std::abs(length(CB) - radius) > EPSILON)
    {
        throw std::runtime_error("Points are not on the circle");
    }

    // 起止角
    double angA = std::atan2(CA[1], CA[0]);
    double angB = std::atan2(CB[1], CB[0]);

    // 中心角
    double theta = angB - angA;
    if (ccw)
    {
        if (theta < 0)
            theta += 2.0 * PI;
    }
    else
    {
        if (theta > 0)
            theta -= 2.0 * PI;
    }

    // CAD bugle
    bulge = std::tan(theta / 4.0);

    result.push_back({A[0], A[1], bulge});
    result.push_back({B[0], B[1], 0.0});

    return result;
}

// 从CAD多线段转化出两个点、圆心和半径
std::tuple<Point2D, Point2D, Point2D, double, bool> fromPolyLineCAD(const PolyLineCAD &P)
{
    Point2D polylineBeg;
    Point2D polylineEnd;
    Point2D center;
    double radius;
    bool ccw;

    polylineBeg = {P[0][0], P[0][1]};
    polylineEnd = {P[1][0], P[1][1]};

    double bulge = P[0][2];
    if (std::abs(bulge) < EPSILON)
        throw std::runtime_error("bulge = 0, this is a line segment");

    double lenAB = length(polylineEnd - polylineBeg);
    if (lenAB < EPSILON)
        throw std::runtime_error("lenAB = 0,this is a line");

    // 中心角
    double theta = 4 * std::atan(bulge);
    double absTheta = std::abs(theta);

    // 半径
    radius = lenAB / (2 * std::sin(absTheta / 2));

    // 弦中点
    Point2D M = (polylineBeg + polylineEnd) * 0.5;

    // 单位垂直方向
    Point2D unitPerp = perp(polylineEnd - polylineBeg);
    unitPerp = unitPerp * (1 / length(unitPerp));

    // bulge决定左右
    if (bulge < 0)
        unitPerp = unitPerp * (-1);

    double h = radius * std::cos(absTheta / 2.0);

    // 圆心
    center = M + unitPerp * h;

    // 是否逆时针
    ccw = (bulge > 0);

    return std::make_tuple(polylineBeg, polylineEnd, center, radius, ccw);
}

MultiPoints2D mirrorMultiPoints2DAlongX(const MultiPoints2D &mPts, double mirrorX)
{
    MultiPoints2D result = mPts;
    for (auto &pt : result)
    {
        pt[0] = -pt[0] + 2 * mirrorX;
    }
    std::reverse(result.begin(), result.end());

    return result;
}
