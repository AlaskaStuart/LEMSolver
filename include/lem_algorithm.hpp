#ifndef SLOPE_LEM_HPP_
#define SLOPE_LEM_HPP_

#pragma warning(push)
#pragma warning(disable : 26495)

#include <Eigen/Dense>
#include <map>
#include <vector>

#pragma warning(pop)

using VectorXd_map = std::map<std::tuple<int, int, int>, Eigen::VectorXd>;
using MatrixXd_map = std::map<std::tuple<int, int, int>, Eigen::MatrixXd>;
using Stratum_map = std::map<int, std::pair<Eigen::MatrixXd, Eigen::MatrixXd>>;
// using PolyLine = Eigen::MatrixXd;
// using Point = Eigen::Vector2d;

std::pair<Eigen::MatrixXd, Eigen::MatrixXd> setSlipEntryAndExitRange(const Eigen::MatrixXd &slope_shape);

VectorXd_map setArcSlips(const Eigen::MatrixXd &entry_points, const Eigen::MatrixXd &exit_points, const Eigen::MatrixXd &slope_shape, int &slips_num);

Eigen::MatrixXd setSlicesWithSingleStrata(const Eigen::VectorXd &entry_point, const Eigen::VectorXd &exit_point, Eigen::MatrixXd intermediate_slope_shape, const Eigen::VectorXd arc_slip, const int &slices_num);
Eigen::MatrixXd setSlicesWithStratum(const Eigen::MatrixXd &slices_geos, const Stratum_map &stratum_map);

double calculateSliceStrataY(const Eigen::MatrixXd &strata_geos, double slice_x);
std::vector<double> calculateIntersectionPointAndTValue(const std::vector<double> &slice_line, Eigen::MatrixXd &strata_line);

double calculateFSOrdinary(const Eigen::MatrixXd &slices_stratum_geos, const std::map<int, Eigen::VectorXd> &stratum_args);
double calculateFSBishop(const Eigen::MatrixXd &slices_geos, const std::map<int, Eigen::VectorXd> &stratum_args);
double calculateFSJanbu(const Eigen::MatrixXd &slices_geos, const std::map<int, Eigen::VectorXd> &stratum_args);
double calculateFSSpencer(const Eigen::MatrixXd &slices_geos, const std::map<int, Eigen::VectorXd> &stratum_args);
double calculateFSMorgensternPrice(const Eigen::MatrixXd &slices_geos, const std::map<int, Eigen::VectorXd> &stratum_args);

#endif