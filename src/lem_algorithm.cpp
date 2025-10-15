#include "lem_algorithm.hpp"

#include "utils.hpp"
// #include "config.hpp"

double calculateSliceStrataY(const Eigen::MatrixXd &strata_geos,
                             double slice_x)
{
    // 计算切片与地层的交点y坐标
    // strata_geos: 地层的几何形状，第一列为x坐标，第二列为y坐标
    // slice_x: 切片的x坐标
    int strata_rows = strata_geos.rows();
    double slice_strata_y = std::nan(""); // 初始化为NaN
    if (strata_rows < 2)
    {
        throw std::runtime_error("Strata geometry must have at least two points.");
    }
    for (int i = 0; i < strata_rows - 1; i++)
    {
        if (slice_x >= strata_geos(i, 0) && slice_x <= strata_geos(i + 1, 0))
        {
            // 线性插值
            double slope = (strata_geos(i + 1, 1) - strata_geos(i, 1)) /
                           (strata_geos(i + 1, 0) - strata_geos(i, 0));
            slice_strata_y =
                strata_geos(i, 1) + slope * (slice_x - strata_geos(i, 0));
            break;
        }
    }
    return slice_strata_y;
}

std::vector<double> calculateIntersectionPointAndTValue(
    const std::vector<double> &slice_line,
    Eigen::MatrixXd &strata_line)
{
    // 从左至右
    // 从上至下
    std::vector<double> result = {std::nan(""), std::nan(""), std::nan("")};

    double x_a = slice_line[0];
    double y_a = slice_line[1];
    double x_b = slice_line[2];
    double y_b = slice_line[3];

    int strata_line_len = strata_line.rows();
    for (int idx_strata_line = 0; idx_strata_line < strata_line_len - 1;
         idx_strata_line++)
    {
        double x_c = strata_line(idx_strata_line, 0);
        double y_c = strata_line(idx_strata_line, 1);
        double x_d = strata_line(idx_strata_line + 1, 0);
        double y_d = strata_line(idx_strata_line + 1, 1);

        double t_cd_x;
        double t_ab_y;

        double det = (x_a - x_b) * (y_c - y_d) - (y_a - y_b) * (x_c - x_d);
        double x_p = ((x_a * y_b - y_a * x_b) * (x_c - x_d) -
                      (x_c * y_d - y_c * x_d) * (x_a - x_b)) /
                     det;
        double y_p = ((x_a * y_b - y_a * x_b) * (y_c - y_d) -
                      (x_c * y_d - y_c * x_d) * (y_a - y_b)) /
                     det;

        t_cd_x = (x_p - x_c) / (x_d - x_c);
        if (t_cd_x < 1 || t_cd_x > 0)
        {
            t_ab_y = (y_p - y_a) / (y_b - y_a);

            result[0] = x_p;
            result[1] = y_p;
            result[2] = t_ab_y;
            break;
        }
    }
    return result;
}

std::pair<Eigen::MatrixXd, Eigen::MatrixXd> setSlipEntryAndExitRange(
    const Eigen::MatrixXd &slope_shape)
{
    // TODO: 现阶段只考虑入口出口为2x2的矩阵，未涉及更复杂的情况。
    Eigen::MatrixXd left_range = slope_shape.topRows(2);
    Eigen::MatrixXd right_range = slope_shape.bottomRows(2);

    return std::make_pair(right_range, left_range);
}

VectorXd_map setArcSlips(const Eigen::MatrixXd &entry_points,
                         const Eigen::MatrixXd &exit_points,
                         const Eigen::MatrixXd &slope_shape,
                         int &slips_num)
{
    VectorXd_map arc_slips_map;

    for (int entry_point_idx = 0; entry_point_idx < entry_points.rows();
         entry_point_idx++)
    {
        for (int exit_point_idx = 0; exit_point_idx < exit_points.rows();
             exit_point_idx++)
        {
            // 获取入口和出口点
            // 两点的连线段作为弦
            Eigen::VectorXd entry_point = entry_points.row(entry_point_idx);
            Eigen::VectorXd exit_point = exit_points.row(exit_point_idx);

            Eigen::VectorXd chord_midpoint = (entry_point + exit_point) / 2.0;

            double chord_length = (entry_point - exit_point).norm();
            double chord_angle = std::atan2(entry_point(1) - exit_point(1),
                                            entry_point(0) - exit_point(0));

            double init_circum_to_chord =
                calculateCircumToChordDistance(chord_length, chord_angle);

            for (int slip_idx = 0; slip_idx < slips_num; slip_idx++)
            {
                std::tuple<int, int, int> arc_slip_key =
                    std::make_tuple(entry_point_idx, exit_point_idx, slip_idx);

                double circum_to_chord =
                    1.1 * init_circum_to_chord * (1 - slip_idx / double(slips_num));
                double radius =
                    calculateRadiusFromChordAndCircum(chord_length, circum_to_chord);
                Eigen::VectorXd center = calculateCenterFromRadiusAndChord(
                    chord_midpoint, chord_angle, radius, circum_to_chord);

                Eigen::VectorXd arc_slip_geos(center.size() + 1);

                // TODO:检查圆弧是否于斜坡相交
                bool is_intersect = false;

                Eigen::MatrixXd interm_slope_geos =
                    cutFromPolyLine(slope_shape, entry_point, exit_point);

                is_intersect =
                    isLineIntersectedCircle(interm_slope_geos, center, radius);

                if (is_intersect)
                {
                    arc_slip_geos.setConstant(std::nan(""));
                }
                else
                {
                    arc_slip_geos << center(0), center(1), radius;
                }

                arc_slips_map[arc_slip_key] = arc_slip_geos;
            }
        }
    }
    return arc_slips_map;
}

Eigen::MatrixXd setSlicesWithSingleStrata(
    const Eigen::VectorXd &entry_point,
    const Eigen::VectorXd &exit_point,
    Eigen::MatrixXd intermediate_slope_shape,
    const Eigen::VectorXd arc_slip,
    const int &slices_num)
{
    double x_c = arc_slip(0);
    double y_c = arc_slip(1);
    double r_c = arc_slip(2);

    double entry_x = entry_point(0);
    double exit_x = exit_point(0);

    Eigen::VectorXd slice_x =
        Eigen::VectorXd::LinSpaced(slices_num + 1, exit_x, entry_x);

    Eigen::VectorXd slice_y_top = Eigen::VectorXd::Zero(slices_num + 1);
    Eigen::VectorXd slice_y_bot = Eigen::VectorXd::Zero(slices_num + 1);

    // initialize the top and bottom y-coordinates of the slices
    slice_y_top(0) = exit_point(1);
    slice_y_top(slices_num) = entry_point(1);

    // 计算顶部y坐标
    for (int idx_slice = 1; idx_slice < slices_num + 1; idx_slice++)
    {
        for (int idx_slp = 0; idx_slp < intermediate_slope_shape.rows() - 1;
             idx_slp++)
        {
            Eigen::VectorXd p1 = intermediate_slope_shape.row(idx_slp);
            Eigen::VectorXd p2 = intermediate_slope_shape.row(idx_slp + 1);

            if (slice_x(idx_slice) > p1(0) && slice_x(idx_slice) <= p2(0))
            {
                // 线性插值
                double slope = (p2(1) - p1(1)) / (p2(0) - p1(0));
                double y_val = p1(1) + slope * (slice_x(idx_slice) - p1(0));
                slice_y_top(idx_slice) = y_val;
                break;
            }
        }
    }
    // 计算底部y坐标
    // slice_y_bot(0) = exit_point(1);
    // slice_y_bot(slices_num) = entry_point(1);
    Eigen::ArrayXd dx = x_c - slice_x.array();
    Eigen::ArrayXd dy = (r_c * r_c - dx.square()).sqrt();
    // 将arrayxd转换成vectorxd

    slice_y_bot = (y_c - dy).matrix();

    // 第一列: x
    // 第二列: y_top
    // 第三列: y_bot
    Eigen::MatrixXd slices(slice_x.size(), 3);
    slices.col(0) = slice_x;
    slices.col(1) = slice_y_top;
    slices.col(2) = slice_y_bot;

    return slices;
}

Eigen::MatrixXd setSlicesWithStratum(const Eigen::MatrixXd &slices_geos,
                                     const Stratum_map &stratum_map)
{
    Eigen::VectorXd slices_x = slices_geos.col(0);
    Eigen::VectorXd slices_y_top = slices_geos.col(1);
    Eigen::VectorXd slices_y_bot = slices_geos.col(2);

    int stratum_num = stratum_map.size();
    int slices_num = slices_geos.rows();

    // return result and initial it.

    Eigen::MatrixXd slices_with_stratum(slices_num, 2 * stratum_num + 3);
    slices_with_stratum.setConstant(std::nan(""));

    slices_with_stratum.col(0) = slices_x;
    slices_with_stratum.col(1) = slices_y_top;
    slices_with_stratum.col(2) = slices_y_bot;

    // 忽略slices的第一行和最后一行
    for (int idx_slice = 1; idx_slice < slices_num - 1; idx_slice++)
    {
        double ith_slice_x = slices_x(idx_slice);
        double ith_slice_y_top = slices_y_top(idx_slice);
        double ith_slice_y_bot = slices_y_bot(idx_slice);

        // std::vector<double> ith_slice_line = { ith_slice_x, ith_slice_y_top,
        // ith_slice_x, ith_slice_y_bot };

        // 计算第idx_slice个切片与各个岩层的交点
        for (int idx_stratum = 0; idx_stratum < stratum_num; idx_stratum++)
        {
            Eigen::MatrixXd jth_strata_top = stratum_map.at(idx_stratum).first;
            Eigen::MatrixXd jth_strata_bot = stratum_map.at(idx_stratum).second;

            double jth_strata_top_y =
                calculateSliceStrataY(jth_strata_top, ith_slice_x);

            double jth_strata_bot_y =
                calculateSliceStrataY(jth_strata_bot, ith_slice_x);

            // 如果交点不存在，则跳过
            if (std::isnan(jth_strata_top_y) || std::isnan(jth_strata_bot_y))
            {
                continue;
            }
            else
            {
                double t_top = (jth_strata_top_y - ith_slice_y_top) /
                               (ith_slice_y_bot - ith_slice_y_top);

                double t_bot = (jth_strata_bot_y - ith_slice_y_top) /
                               (ith_slice_y_bot - ith_slice_y_top);

                if (t_bot > 0 && t_top < 1)
                {
                    double top_val = std::min(ith_slice_y_top, jth_strata_top_y);
                    double bot_val = std::max(ith_slice_y_bot, jth_strata_bot_y);

                    slices_with_stratum(idx_slice, 2 * idx_stratum + 3) = top_val;
                    slices_with_stratum(idx_slice, 2 * idx_stratum + 4) = bot_val;
                }
            }
        }
    }
    // 第一行和最后一行的y坐标
    for (int idx_stratum = 0; idx_stratum < stratum_num; idx_stratum++)
    {
        Eigen::MatrixXd jth_strata_top = stratum_map.at(idx_stratum).first;
        Eigen::MatrixXd jth_strata_bot = stratum_map.at(idx_stratum).second;

        double jth_strata_top_y_entry =
            calculateSliceStrataY(jth_strata_top, slices_x(0));

        double jth_strata_bot_y_entry =
            calculateSliceStrataY(jth_strata_bot, slices_x(0));

        if (jth_strata_top_y_entry > slices_y_top(0) &&
            jth_strata_bot_y_entry <= slices_y_top(0))
        {
            slices_with_stratum(0, 2 * idx_stratum + 3) = slices_y_top(0);
            slices_with_stratum(0, 2 * idx_stratum + 4) = slices_y_top(0);
            break;
        }
    }
    for (int idx_stratum = 0; idx_stratum < stratum_num; idx_stratum++)
    {
        Eigen::MatrixXd jth_strata_top = stratum_map.at(idx_stratum).first;
        Eigen::MatrixXd jth_strata_bot = stratum_map.at(idx_stratum).second;

        double jth_strata_top_y_exit =
            calculateSliceStrataY(jth_strata_top, slices_x(slices_x.size() - 1));

        double jth_strata_bot_y_exit =
            calculateSliceStrataY(jth_strata_bot, slices_x(slices_x.size() - 1));

        if (jth_strata_top_y_exit >= slices_y_top(slices_y_top.size() - 1) &&
            jth_strata_bot_y_exit <= slices_y_top(slices_y_top.size() - 1))
        {
            slices_with_stratum(slices_num - 1, 2 * idx_stratum + 3) =
                slices_y_top(slices_y_top.size() - 1);

            slices_with_stratum(slices_num - 1, 2 * idx_stratum + 4) =
                slices_y_top(slices_y_top.size() - 1);
            break;
        }
    }
    return slices_with_stratum;
}

// TODO: 保证slices_geos.cols() 和stratum_args.size()的大小一致
// TODO: make sure that `slices_geos.cols()` equals to the `stratum_args.size()`
double calculateFSOrdinary(const Eigen::MatrixXd &slices_stratum_geos,
                           const std::map<int, Eigen::VectorXd> &stratum_args)
{
    double pushing_force = 0;
    double resistance_force = 0;
    // Eigen::VectorXd slices_x = slices_geos.col(0);

    if (slices_stratum_geos.cols() == 3)
    {
        // single strata
        // 单一岩土层
        // TODO
        return -1.0;
    }
    else
    {
        // stratum
        // 多岩层
        // n+1 slices, m stratum
        // size of matrix = (n+1, 2*m+3)
        for (int slice_row_idx = 0; slice_row_idx < slices_stratum_geos.rows() - 2;
             slice_row_idx++)
        {
            Eigen::VectorXd slice_left = slices_stratum_geos.row(slice_row_idx);
            Eigen::VectorXd slice_right = slices_stratum_geos.row(slice_row_idx + 1);

            // arrange `block_x` to be like 1,2,1,2
            // `block_x`顺序安排 1,2,1,2
            double x_i_left = slice_left(0);
            double x_i_right = slice_right(0);

            Eigen::VectorXd block_x(4);
            block_x << x_i_left, x_i_left, x_i_right, x_i_right;

            // calculate the weight of slice
            double slice_weight = 0;
            for (int slice_col_idx = 3;
                 slice_col_idx < slices_stratum_geos.cols() - 1;
                 slice_col_idx = slice_col_idx + 2)
            {
                Eigen::MatrixXd submat(2, 2);
                submat = slices_stratum_geos.block(slice_row_idx, slice_col_idx, 2, 2);
                if (submat.array().isNaN().all())
                {
                    continue;
                }

                double y_ij_left_top = submat(0, 0);
                double y_ij_left_bot = submat(0, 1);
                double y_ij_right_top = submat(1, 0);
                double y_ij_right_bot = submat(1, 1);

                if ((std::isnan(y_ij_left_top)) || (std::isnan(y_ij_left_bot)))
                {
                    int last_left_idx = getIndexOfLastNonNaN(slice_left, slice_col_idx);
                    if (last_left_idx < 1)
                    {
                        throw std::runtime_error("Last left index is less than 1. Please check the slices_with_stratum matrix.");
                    }
                    else if (last_left_idx == 2)
                    {
                        int idx_down = slice_col_idx + 2;
                        for (; idx_down < slices_stratum_geos.cols(); idx_down = idx_down + 2)
                        {
                            if (!std::isnan(slice_left(idx_down)))
                            {
                                last_left_idx = idx_down;
                                break;
                            }
                        }
                    }

                    y_ij_left_top = slice_left(last_left_idx);
                    y_ij_left_bot = slice_left(last_left_idx);
                }

                if ((std::isnan(y_ij_right_top)) || (std::isnan(y_ij_right_bot)))
                {
                    int last_right_idx = getIndexOfLastNonNaN(slice_right, slice_col_idx);
                    if (last_right_idx < 1)
                    {
                        throw std::runtime_error("Last right index is less than 1. Please check the slices_with_stratum matrix.");
                    }
                    else if (last_right_idx == 2)
                    {
                        int idx_down = slice_col_idx + 2;
                        for (; idx_down < slices_stratum_geos.cols(); idx_down = idx_down + 2)
                        {
                            if (!std::isnan(slice_right(idx_down)))
                            {
                                last_right_idx = idx_down;
                                break;
                            }
                        }
                    }
                    y_ij_right_top = slice_right(last_right_idx);
                    y_ij_right_bot = slice_right(last_right_idx);
                }

                Eigen::VectorXd block_y(4);
                block_y << y_ij_left_top, y_ij_left_bot, y_ij_right_top, y_ij_right_bot;

                double block_area = calculateAreaOfQuad(block_x, block_y);
                int stratum_idx = (slice_col_idx - 3) / 2;
                double gamma = stratum_args.at(stratum_idx)(0);

                slice_weight += block_area * gamma;
            }
            int slice_left_bot_idx =
                getIndexOfLastNonNaN(slice_left, slice_left.size() - 1);

            int slice_right_bot_idx =
                getIndexOfLastNonNaN(slice_right, slice_right.size() - 1);

            int phi_and_c_idx = (std::max(slice_left_bot_idx, slice_right_bot_idx) - 3) / 2;

            double cohension = stratum_args.at(phi_and_c_idx)(1);
            double friction_angle = stratum_args.at(phi_and_c_idx)(2);

            double y_bot_left = slice_left(slice_left_bot_idx);
            double y_bot_right = slice_right(slice_right_bot_idx);

            double bot_length = std::sqrt(std::pow((y_bot_right - y_bot_left), 2) +
                                          std::pow((x_i_right - x_i_left), 2));
            double bot_angle =
                std::atan2(y_bot_right - y_bot_left, x_i_right - x_i_left);

            double slice_cohension_force = cohension * bot_length;

            double slice_friction =
                slice_weight * std::cos(bot_angle) * std::tan(friction_angle);

            double slice_resistance_force = slice_cohension_force + slice_friction;
            double slice_pushing_force = slice_weight * std::sin(bot_angle);
            if (slice_pushing_force < 0)
            {
                resistance_force += (resistance_force - pushing_force);
            }
            else
            {
                resistance_force += slice_resistance_force;
                pushing_force += slice_pushing_force;
            }
        }
        return resistance_force / pushing_force;
    }
}

double calculateFSBishop(const Eigen::MatrixXd &slices_geos,
                         const std::map<int, Eigen::VectorXd> &stratum_args)
{
    return -1.0;
}

double calculateFSJanbu(const Eigen::MatrixXd &slices_geos,
                        const std::map<int, Eigen::VectorXd> &stratum_args)
{
    return -1.0;
}

double calculateFSSpencer(const Eigen::MatrixXd &slices_geos,
                          const std::map<int, Eigen::VectorXd> &stratum_args)
{
    return -1.0;
}

double calculateFSMorgensternPrice(
    const Eigen::MatrixXd &slices_geos,
    const std::map<int, Eigen::VectorXd> &stratum_args)
{
    return -1.0;
}
