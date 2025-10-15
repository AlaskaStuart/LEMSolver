#include <iomanip>
#include <iostream>

#include "config.hpp"
#include "lem_algorithm.hpp"
#include "utils.hpp"

class SlopeAnalysis
{
private:
    int entry_exit_points_num_;
    int slips_num_;
    int slices_num_;

    // input data
    Eigen::MatrixXd slope_geos_;
    Stratum_map stratum_map_;
    std::map<int, Eigen::VectorXd> stratum_args_;

    // processed data
    Eigen::MatrixXd entry_range_;
    Eigen::MatrixXd exit_range_;

    Eigen::MatrixXd entry_points_set_;
    Eigen::MatrixXd exit_points_set_;

    VectorXd_map arc_slips_set_;
    MatrixXd_map slices_geos_set_;
    MatrixXd_map slices_with_stratum_set_;
    std::map<std::tuple<int, int, int>, float> analysis_results_;

public:
    SlopeAnalysis(int entry_exit_points_num, int slips_num, int slices_num,
                  const Eigen::MatrixXd &slope_geos,
                  const Stratum_map &stratum_map,
                  const std::map<int, Eigen::VectorXd> &stratum_args)
        : entry_exit_points_num_(entry_exit_points_num),
          slips_num_(slips_num),
          slices_num_(slices_num),
          slope_geos_(slope_geos),
          stratum_map_(stratum_map),
          stratum_args_(stratum_args)
    {
        std::pair<Eigen::MatrixXd, Eigen::MatrixXd> entry_exit_ranges =
            setSlipEntryAndExitRange(slope_geos_);

        entry_points_set_ =
            interpolateMatrix(entry_exit_ranges.first, entry_exit_points_num_);
        exit_points_set_ =
            interpolateMatrix(entry_exit_ranges.second, entry_exit_points_num_);

        arc_slips_set_ = setArcSlips(entry_points_set_, exit_points_set_,
                                     slope_geos_, slips_num_);

        slices_geos_set_ =
            generateSlicesGeos(entry_points_set_, exit_points_set_, slope_geos_,
                               arc_slips_set_, slips_num_);
        slices_with_stratum_set_ =
            generateSlicesStratum(slices_geos_set_, stratum_map_);
    }

    Eigen::MatrixXd getSlopeGeos() const { return slope_geos_; }
    Stratum_map getStratumMap() const { return stratum_map_; }
    std::map<int, Eigen::VectorXd> getStratumArgs() const { return stratum_args_; }
    Eigen::MatrixXd getEntryPointsSet() const { return entry_points_set_; }
    Eigen::MatrixXd getExitPointsSet() const { return exit_points_set_; }
    VectorXd_map getArcSlips() const { return arc_slips_set_; }
    MatrixXd_map getSlicesGeos() const { return slices_geos_set_; }
    MatrixXd_map getSlicesWithStratum() const { return slices_with_stratum_set_; }

    MatrixXd_map generateSlicesGeos(const Eigen::MatrixXd &entry_points,
                                    const Eigen::MatrixXd &exit_points,
                                    const Eigen::MatrixXd &slope_shape,
                                    const VectorXd_map &arc_slips_set,
                                    int &slips_num);

    MatrixXd_map generateSlicesStratum(const MatrixXd_map &slices_set,
                                       const Stratum_map &stratum_map);

    std::map<std::tuple<int, int, int>, double> startAnalysis(const MatrixXd_map &slices_with_stratum_set,
                                                              std::map<int, Eigen::VectorXd> stratum_args, std::string methodname = "Ordinary");
};

MatrixXd_map SlopeAnalysis::generateSlicesGeos(
    const Eigen::MatrixXd &entry_points, const Eigen::MatrixXd &exit_points,
    const Eigen::MatrixXd &slope_shape, const VectorXd_map &arc_slips_set,
    int &slips_num)
{
    MatrixXd_map slices_geos_set;

    for (const auto &ith_arc_slip : arc_slips_set)
    {
        auto ith_arc_slip_key = ith_arc_slip.first;
        Eigen::MatrixXd ith_arc_slip_args = ith_arc_slip.second;

        int entry_idx = std::get<0>(ith_arc_slip_key);
        int exit_idx = std::get<1>(ith_arc_slip_key);
        int slip_idx = std::get<2>(ith_arc_slip_key);
        Eigen::MatrixXd ith_slice_geos(slips_num + 1, 3);
        if (ith_arc_slip_args.array().isNaN().all())
        {
            ith_slice_geos.setConstant(std::nan(""));
        }
        else
        {
            Eigen::VectorXd entry_point = entry_points.row(entry_idx);
            Eigen::VectorXd exit_point = exit_points.row(exit_idx);

            Eigen::MatrixXd interm_slope_geos =
                cutFromPolyLine(slope_shape, entry_point, exit_point);

            ith_slice_geos =
                setSlicesWithSingleStrata(entry_point, exit_point, interm_slope_geos,
                                          ith_arc_slip_args, slips_num);
        }
        slices_geos_set[ith_arc_slip_key] = ith_slice_geos;
    }
    return slices_geos_set;
}

MatrixXd_map SlopeAnalysis::generateSlicesStratum(
    const MatrixXd_map &slices_set, const Stratum_map &stratum_map)
{
    MatrixXd_map slices_with_stratum;

    for (auto ith_slices : slices_set)
    {
        auto ith_slices_key = ith_slices.first;
        auto ith_slices_geos = ith_slices.second;

        Eigen::MatrixXd ith_slices_with_stratum(ith_slices_geos.rows(),
                                                stratum_map.size());

        if (ith_slices_geos.array().isNaN().all())
        {
            ith_slices_with_stratum.setConstant(std::nan(""));
        }
        else
        {
            ith_slices_with_stratum =
                setSlicesWithStratum(ith_slices_geos, stratum_map);
        }
        slices_with_stratum[ith_slices_key] = ith_slices_with_stratum;
    }
    return slices_with_stratum;
}

// TODO: multi-thread
std::map<std::tuple<int, int, int>, double>
SlopeAnalysis::startAnalysis(const MatrixXd_map &
                                 slices_with_stratum_set,
                             std::map<int, Eigen::VectorXd> stratum_args, std::string methodname)
{
    std::map<std::tuple<int, int, int>, double> analysis_results;

    if (methodname == "Ordinary")
    {
        for (const auto &ith_slices_stratum : slices_with_stratum_set)
        {
            auto slices_key = ith_slices_stratum.first;

            Eigen::MatrixXd slices_geos = ith_slices_stratum.second;

            double result = calculateFSOrdinary(slices_geos, stratum_args);
            analysis_results[slices_key] = result;
        }
    }

    else if (methodname == "Bishop")
    {
        for (const auto &slices_stratum : slices_with_stratum_set_)
        {
            auto slices_key = slices_stratum.first;
            Eigen::MatrixXd slices_geos = slices_stratum.second;
            double result = calculateFSBishop(slices_geos, stratum_args);
            analysis_results[slices_key] = result;
        }
    }
    else if (methodname == "Janbu")
    {
        for (const auto &slices_stratum : slices_with_stratum_set_)
        {
            auto slices_key = slices_stratum.first;
            Eigen::MatrixXd slices_geos = slices_stratum.second;
            double result = calculateFSJanbu(slices_geos, stratum_args);
            analysis_results[slices_key] = result;
        }
    }
    else if (methodname == "Spencer")
    {
        for (const auto &slices_stratum : slices_with_stratum_set_)
        {
            auto slices_key = slices_stratum.first;
            Eigen::MatrixXd slices_geos = slices_stratum.second;
            double result = calculateFSSpencer(slices_geos, stratum_args);
            analysis_results[slices_key] = result;
        }
    }
    else if (methodname == "MorgensternPrice")
    {
        for (const auto &slices_stratum : slices_with_stratum_set_)
        {
            auto slices_key = slices_stratum.first;
            Eigen::MatrixXd slices_geos = slices_stratum.second;
            double result = calculateFSMorgensternPrice(slices_geos,
                                                        stratum_args);
            analysis_results[slices_key] = result;
        }
    }

    return analysis_results;
}