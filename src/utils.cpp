#pragma warning(push)
#pragma warning(disable : 26495)
#pragma warning(pop)

#include "utils.hpp"
// #include "config.hpp"

double roundToXDigits(double value)
{
    double factor = 10000.0;
    return std::round(value * factor) / factor;
}

// Eigen::VectorXd calculateIntersectionPoint(std::vector<double> line_a, std::vector<double> line_b)
// {

//     // 如果两者相交，则返回交点坐标，否则返回NaN
//     // a为顶，b为底，同时返回交点p到a的距离与ab段的比值，可以为负值 t=|ap|/|ab|
//     double x_a = line_a[0];
//     double y_a = line_a[1];
//     double x_b = line_a[2];
//     double y_b = line_a[3];

//     double x_c = line_b[0];
//     double y_c = line_b[1];
//     double x_d = line_b[2];
//     double y_d = line_b[3];

//     double x_p;
//     double y_p;
//     Eigen::VectorXd intersection_point(3);
//     // calculate the intersection point of two lines
//     double det = (x_a - x_b) * (y_c - y_d) - (y_a - y_b) * (x_c - x_d);
//     if (det == 0)
//     {
//         intersection_point.setConstant(std::nan(""));
//     }
//     else
//     {
//         x_p = ((x_a * y_b - y_a * x_b) * (x_c - x_d) - (x_c * y_d - y_c * x_d) * (x_a - x_b)) / det;
//         y_p = ((x_a * y_b - y_a * x_b) * (y_c - y_d) - (x_c * y_d - y_c * x_d) * (y_a - y_b)) / det;
//     }
//     double t_a = (y_p - y_a) / (y_b - y_a);
//     double t_b = (x_p - x_c) / (x_d - x_c);
//     if ((t_a > 1 || t_a < 0) || (t_b > 1 || t_b < 0))
//     {
//         // return NaN if the intersection point is not on the line segment
//         intersection_point.setConstant(std::nan(""));
//     }
//     else
//     {
//         intersection_point << x_p, y_p;
//     }
//     return intersection_point;
// }

Eigen::MatrixXd cutFromPolyLine(const Eigen::MatrixXd &polyline, Eigen::VectorXd start_point, Eigen::VectorXd end_point)
{
    Eigen::MatrixXd cut_polyline;
    // switch the start and end point if the start point is on the right of the end point
    if (start_point(0) > end_point(0))
    {
        std::swap(start_point, end_point);
    }

    int start_index = 0;
    int end_index = polyline.rows() - 1;

    double left_x = start_point(0);
    double right_x = end_point(0);

    while (start_index < polyline.rows() - 1)
    {
        if (polyline(start_index, 0) <= left_x && polyline(start_index + 1, 0) > left_x)
        {
            break;
        }
        else
        {
            start_index++;
        }
    }
    while (end_index > 0)
    {
        if (polyline(end_index - 1, 0) < right_x && polyline(end_index, 0) >= right_x)
        {
            break;
        }
        else
        {
            end_index--;
        }
    }

    if (end_index < start_index)
    {
        throw std::runtime_error("End index is less than start index. Please check the input polyline.");
    }
    else if (start_index == end_index)
    {
        cut_polyline.resize(2, polyline.cols());
        cut_polyline.row(0) = start_point;
        cut_polyline.row(1) = end_point;
    }
    else
    {
        cut_polyline = polyline.block(start_index, 0, end_index - start_index + 1, polyline.cols());
        cut_polyline.row(0) = start_point;
        cut_polyline.row(cut_polyline.rows() - 1) = end_point;
    }
    return cut_polyline;
}

Eigen::MatrixXd interpolateMatrix(const Eigen::MatrixXd &mat, int num_points)
{

    Eigen::MatrixXd result(num_points, mat.cols());

    int mat_size = mat.rows();
    if (mat_size < 2)
    {
        // error
        throw std::runtime_error("Matrix size is less than 2.");
    }
    else if (mat_size == 2)
    {
        result.col(0) = Eigen::VectorXd::LinSpaced(num_points, double(mat(0, 0)), double(mat(1, 0)));
        result.col(1) = Eigen::VectorXd::LinSpaced(num_points, double(mat(0, 1)), double(mat(1, 1)));
    }
    else
    {
        Eigen::VectorXd result_x = Eigen::VectorXd::LinSpaced(num_points, double(mat(0, 0)), double(mat(1, 0)));

        Eigen::VectorXd result_y(mat_size);

        for (int i = 0; i < num_points; i++)
        {
            double x = result_x(i);
            // linear interpolation
            for (int j = 0; j < mat_size - 1; j++)
            {
                if (x >= mat(j, 0) && x <= mat(j + 1, 0))
                {
                    double slope = (mat(j + 1, 1) - mat(j, 1)) / (mat(j + 1, 0) - mat(j, 0));
                    result_y(i) = mat(j, 1) + slope * (x - mat(j, 0));
                    break;
                }
            }
        }

        result.col(0) = result_x;
        result.col(1) = result_y;
    }

    return result;
}

double calculateCircumToChordDistance(double chord_length, double chord_angle)
{

    double radius = chord_length / (2.0 * std::cos(chord_angle));
    double circum_to_chord = std::sqrt(radius * radius - (chord_length / 2.0) * (chord_length / 2.0));
    return circum_to_chord;
}

double calculateRadiusFromChordAndCircum(double chord_length, double circum_to_chord)
{
    double radius = (chord_length * chord_length * 0.25 + circum_to_chord * circum_to_chord) / (2 * circum_to_chord);
    return radius;
}

Eigen::VectorXd calculateCenterFromRadiusAndChord(const Eigen::VectorXd chord_midpoint, double chord_angle, double radius, double circum_to_chord)
{

    Eigen::VectorXd offset(2);
    double center_to_chord = radius - circum_to_chord;
    offset << center_to_chord * std::cos(chord_angle + M_PI / 2.0), center_to_chord * std::sin(chord_angle + M_PI / 2.0);
    Eigen::VectorXd center = chord_midpoint + offset;

    return center;
}

bool isLineIntersectedCircle(const Eigen::MatrixXd &pline, const Eigen::VectorXd &center, double R)
{
    bool is_intersect = false;

    int pline_rows = pline.rows();
    if (pline_rows < 2)
    {
        // return true; // 直线至少需要两个点
        is_intersect = true;
    }
    for (int pline_idx = 1; pline_idx < pline_rows - 2; pline_idx++)
    {

        Eigen::VectorXd p1 = pline.row(pline_idx);
        Eigen::VectorXd p2 = pline.row(pline_idx + 1);
        Eigen::VectorXd dir = p2 - p1;

        // 直线参数方程: p = p1 + t * dir
        // 代入圆的方程 ||p - pc||² = R²
        Eigen::Vector2d delta = p1 - center;
        double a = dir.squaredNorm();
        double b = 2 * delta.dot(dir);
        double c = delta.squaredNorm() - R * R;

        // 解二次方程 a*t² + b*t + c = 0
        double discriminant = b * b - 4 * a * c;

        if (discriminant > 0)
        {
            // 设置double的精度为4位有效位数

            double t1 = roundToXDigits((-b + std::sqrt(discriminant)) / (2 * a));
            double t2 = roundToXDigits((-b - std::sqrt(discriminant)) / (2 * a));
            // 检查 t1 或 t2 是否在 [0, 1] 范围内
            is_intersect = (t1 >= 0 && t1 <= 1) || (t2 >= 0 && t2 <= 1);
            if (is_intersect)
            {
                break; // 找到一个交点就可以了
            }
        }
        else if (discriminant == 0)
        {
            double t = -b / (2 * a);
            // 检查 t 是否在 [0, 1] 范围内
            is_intersect = (t >= 0 && t <= 1);
            if (is_intersect)
            {
                break; // 找到一个交点就可以了
            }
        }
        else
        {
            continue;
        }
    }
    return is_intersect;
}

double calculateAreaOfQuad(const Eigen::VectorXd &points_x, const Eigen::VectorXd &points_y)
{
    // P_i =(x_i,y_i)
    // P_4 = P_0
    // area  = 1/2*sum_0^3(x_i*y_(i+1)- x_(i+1)y_i)

    Eigen::VectorXd x_shift = points_x.tail(3).eval();
    x_shift.conservativeResize(4);
    x_shift(3) = points_x(0);

    Eigen::VectorXd y_shift = points_y.tail(3).eval();
    y_shift.conservativeResize(4);
    y_shift(3) = points_y(0);

    return std::abs((points_x.cwiseProduct(y_shift) - x_shift.cwiseProduct(points_y)).sum()) / 2.0;
}

int getIndexOfLastNonNaN(const Eigen::VectorXd &slice_stratum, int idx)
{
    if (slice_stratum.size() == 0)
    {
        throw std::runtime_error("Slice stratum is empty.");
    }
    if (idx < 0 || idx >= slice_stratum.size() - 1)
    {
        idx = slice_stratum.size() - 1;
    }
    int last_non_nan_idx = idx;
    for (; last_non_nan_idx >= 0; last_non_nan_idx--)
    {
        if (!std::isnan(slice_stratum(last_non_nan_idx)))
        {
            break;
        }
    }
    return last_non_nan_idx;
}