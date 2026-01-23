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

// 根据指定的起始点和终止点截取折线段, 需要以指定的起始点开始, 指定的终止点结束
// 实现过程:
// 找到start_index和end_index
// start_index相应的元素其x值要小于start_x, end_index相应的元素其x值要大于end_x
// 这样根据左右两边的index截取出来vector
// 再将start_point和end_point替换新vector的第一个元素和最后一个元素
MultiPoints2D cutFromPolyLine(const MultiPoints2D &polyline,
                              std::array<double, 2> startPoint,
                              std::array<double, 2> endPoint)
{

    MultiPoints2D result;

    int startIdx = 0;
    int endIdx = polyline.size() - 1;

    if (startPoint[0] > endPoint[0])
    {
        std::swap(startPoint, endPoint);
    }
    double startX = startPoint[0];
    double endX = endPoint[0];

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
        result.front() = startPoint;
        result.back() = endPoint;
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

// 不包含线段的起始点和终止点，如果存在其他交点，则返回为true
bool isLineIntersectedCircle(const MultiPoints2D &polyline,
                             const std::array<double, 2> &center,
                             double radius)
{
    bool isIntersected = false;
    int polylinePts = polyline.size();
    if (polylinePts < 2)
    {
        throw std::runtime_error("The number of points is less than 2");
    }
    else
    {
        for (int plIdx = 1; plIdx < polylinePts - 2; plIdx++)
        {
            std::array<double, 2> p1 = polyline[plIdx];
            std::array<double, 2> p2 = polyline[plIdx + 1];

            std::array<double, 2> dir = {
                p2[0] - p1[0],
                p2[1] - p1[1]};

            // p = p1 + t * dir
            // ||p - center||^2 = R^2
            std::array<double, 2> delta = {
                p1[0] - center[0],
                p1[1] - center[1]};

            double a = dir[0] * dir[0] + dir[1] * dir[1];
            double b = 2 * (dir[0] * delta[0] + dir[1] * delta[1]);
            double c = delta[0] * delta[0] + delta[1] * delta[1] - radius * radius;

            // 解二次方程 a*t² + b*t + c = 0
            double discriminant = b * b - 4 * a * c;
            if (discriminant > 0)
            {
                double t1 = (-b + std::sqrt(discriminant)) / (2 * a);
                double t2 = (-b - std::sqrt(discriminant)) / (2 * a);
                // 检查 t1 或 t2 是否在 [0, 1] 范围内
                isIntersected = (t1 >= 0 && t1 <= 1) || (t2 >= 0 && t2 <= 1);
                if (isIntersected)
                {
                    break;
                }
            }
            else if (discriminant == 0)
            {
                double t = -b / (2 * a);
                // 检查 t 是否在 [0, 1] 范围内
                isIntersected = (t >= 0 && t <= 1);
                if (isIntersected)
                {
                    break; // 找到一个交点就可以了
                }
            }
            else
            {
                continue;
            }
        }
        return isIntersected;
    };
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

// 生成两个折线段相交之后、位于相交点以上部分的折线段。
// 返回结果：如果两个线段，相交点之上部分线段合并后的折线段
// 如果两个线段不相交，则返回结果是平均y值较大的折线段
// 考虑的是，可能相交点不止一个，这种情况下尾段不知道该怎么处理
// 而且如果不相交，这两个折线段的上下关系不好判定，阶段性放弃
// @deprecated
// 可能引发bug
// std::optional<PolyLineCAD> getUpperPolyLineIntersection(const MultiPoints2D &P, const MultiPoints2D &Q)
// {
//     PolyLineCAD upperPolyLine;
//     MultiPoints2D intersectionPt;

//     // if ((P.size() == 1) || (Q.size() == 1))
//     // {

//     // }

//     // if (P.size() == 1 and Q.size() > 1)
//     // {
//     //     if (P.front()[1] > (Q.front()[1]))
//     //     {
//     //         return P;
//     //     }
//     //     else
//     //     {
//     //         return Q;
//     //     }
//     // }
//     // else if (Q.size() == 1 and P.size() > 1)
//     // {
//     //     if (Q.front()[1] > (P.front()[1]))
//     //     {
//     //         return Q;
//     //     }
//     //     else
//     //     {
//     //         return P;
//     //     }
//     // }
//     // else if (Q.size() == 1 and P.size() == 1)
//     // {
//     //     if (P.front()[1] > (Q.front()[1]))
//     //     {
//     //         return P;
//     //     }
//     //     else
//     //     {
//     //         return Q;
//     //     }
//     // }

//     for (size_t i = 0; i + 1 < P.size(); i++)
//     {
//         for (size_t j = 0; j + 1 < Q.size(); j++)
//         {
//             Point2D p1 = P[i];
//             Point2D p2 = P[i + 1];

//             Point2D q1 = Q[j];
//             Point2D q2 = Q[j + 1];

//             auto intersection = segmentIntersection(p1, p2, q1, q2);
//             if (intersection)
//             {
//                 const Point2D &intersectionVal = intersection.value();

//                 intersectionPt.push_back(intersectionVal);

//                 if (p1[1] > q1[1])
//                 {
//                     upperPolyLine.insert(upperPolyLine.end(), P.begin(), P.begin() + i);
//                     upperPolyLine.push_back({intersection.value()[0], intersectionVal[1], 0});
//                 }
//                 else if (q1[1] > p1[1])
//                 {
//                     upperPolyLine.insert(upperPolyLine.end(), Q.begin(), Q.begin() + j);
//                     upperPolyLine.push_back({intersectionVal[0], intersectionVal[1], 0});
//                 }

//                 MultiPoints2D rightHalfP;
//                 MultiPoints2D rightHalfQ;
//                 rightHalfP.insert(rightHalfP.end(), P.begin() + i + 1, P.end());
//                 rightHalfQ.insert(rightHalfQ.end(), Q.begin() + j + 1, Q.end());

//                 std::optional<PolyLineCAD> nextPartIntersection = getUpperPolyLineIntersection(rightHalfP, rightHalfQ);
//                 if (nextPartIntersection)
//                 {
//                     upperPolyLine.insert(upperPolyLine.end(), nextPartIntersection.value().begin(), nextPartIntersection.value().end());
//                 }
//                 break;
//             }
//         }
//     }

//     if (intersectionPt.size() == 0)
//     {

//         Point2D P_begin = {P.front()[0], P.front()[1]};
//         Point2D Q_begin = {Q.front()[0], Q.front()[1]};

//         if (length(P_begin) > length(Q_begin))
//         {
//             return P;
//         }
//         else
//         {
//             return Q;
//         }
//     }

//     return upperPolyLine;
// }

// PolyLineCAD的圆弧段与多线段相交，返回相交后的折线段
// P为软弱层、断层、用户指定折线段
// Q为圆弧滑面
std::optional<PolyLineCAD> getUpperPolyLineIntersection(const PolyLineCAD &P, PolyLineCAD &Q)
{
    PolyLineCAD upperBound;

    double radius;
    Point2D center;
    Point2D entryPoint;
    Point2D exitPoint;

    entryPoint = {Q[0][0], Q[0][1]};
    exitPoint = {Q[1][0], Q[1][1]};

    return upperBound;
}