#include "geometry.h"

const std::array<double, 2> &getMidPoint(std::array<double, 2> &entryPoint, std::array<double, 2> &exitPoint)
{
    return {0.5 * (entryPoint[0] + exitPoint[0]), 0.5 * (entryPoint[1] + exitPoint[1])};
}

double calculateLength(std::array<double, 2> &exitPoint, std::array<double, 2> &entryPoint)
{
    return std::sqrt(std::pow(exitPoint[0] - entryPoint[0], 2) + std::pow(exitPoint[1] - entryPoint[1], 2));
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

    double dx = (end - start) / (numPoints - 1);
    for (int i = 0; i < numPoints; i++)
    {
        double x = start + i * dx;
        result.push_back(x);
    }
    return result;
}
// numPoints 为插值后点的数量(包括原来的polyline起止点)，因此numPoints >= 3
MultiPoints2D interpolatedPolyLine(const MultiPoints2D &polyline, int numPoints)
{
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
        double dx = (polyline.front()[0] - polyline.back()[0]) / (numPoints - 1);

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

// 根据弦长和弦度，计算圆周到弦的距离
// double calculateCircumToChordDist(double chord, double chordAngle)
// {
//     double radius = chord / (2.0 * std::cos(chordAngle));
//     double chordToCircum = std::sqrt(radius * radius - (chord / 2.0) * (chord / 2.0));
//     return chordToCircum;
// }
// chordToCircum = 2*chord*std::tan(horizontalAngle);

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
    std::array<double, 2> center;
    double centertoChord = radius - chordtoCircum;
    double radiusX = chordMid[0] - centertoChord * std::cos(angle);
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

// double calculateAreaOfQuad(const MultiPoints2D &pts)
// {
//     // 要求输入 4 个点
//     if (pts.size() != 4)
//         throw std::runtime_error("calculate the area of Quad requires exactly 4 points.");

//     // shoelace formula:
//     // area = 1/2 * Σ (x_i * y_(i+1) - x_(i+1) * y_i)
//     double sum = 0.0;

//     for (size_t i = 0; i < 4; ++i)
//     {
//         size_t j = (i + 1) % 4; // 下一个点（最后一个点的下一个是点0）

//         double xi = pts[i][0];
//         double yi = pts[i][1];
//         double xj = pts[j][0];
//         double yj = pts[j][1];

//         sum += (xi * yj - xj * yi);
//     }

//     return std::abs(sum) / 2.0;
// }

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

// int main()
// {
//     // 定义一个示例折线
//     MultiPoints2D polyline = {
//         {0.0, 0.0},
//         {1.0, 2.0},
//         {2.0, 3.0},
//         {3.0, 5.0},
//         {4.0, 4.0},
//         {5.0, 2.0}};

//     std::array<double, 2> startPoint = {1.5, 2.5};
//     std::array<double, 2> endPoint = {4.5, 3.0};

//     try
//     {
//         MultiPoints2D subLine = cutFromPolyLine(polyline, startPoint, endPoint);
//         std::cout << "Cutted polyline:\n";
//         printPolyLine(subLine);
//     }
//     catch (const std::exception &e)
//     {
//         std::cerr << "Error: " << e.what() << "\n";
//     }

//     return 0;
// }