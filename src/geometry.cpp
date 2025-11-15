#include "geometry.h"

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

// numPoints 为插值后点的数量(包括原来的polyline起止点)，因此numPoints >= 3
MultiPoints2D interpolatedPolyLine(const MultiPoints2D &polyline, int numPoints)
{
    MultiPoints2D result;
    result.reserve(numPoints);
    if (polyline.size() < 2)
    {
        throw std::runtime_error("The number of points is less than 2, cannot operate interpolation");
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
            for (int j = 0; j < polyline.size() - 1; j++)
            {
                if (polyline[j][0] <= x && x <= polyline[j + 1][0])
                {
                    double slope = (polyline[j + 1][1] - polyline[j][1]) / (polyline[j + 1][0] - polyline[j][0]);
                    double y = polyline[j][1] + slope * (x - polyline[j][0]);
                    result.push_back({x, y});
                    break;
                }
            }
        }
    }
    return result;
}

double calculateCircumToChordDist(double chordLength, double chordAngle)
{
    double radius = chordLength / (2.0 * std::cos(chordAngle));
    double circumToChord = std::sqrt(radius * radius - (chordLength / 2.0) * (chordLength / 2.0));
    return circumToChord;
}

double calculateRadiusFromChordAndCircum(double chordLength, double circumToChord)
{
    double radius = (chordLength * chordLength * 0.25 + circumToChord * circumToChord) / (2 * circumToChord);
    return radius;
}

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