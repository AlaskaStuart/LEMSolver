#pragma once

#ifndef SLOPE_UTILS_HPP_
#define SLOPE_UTILS_HPP_

#pragma warning(push)
#pragma warning(disable : 26495)
#include <Eigen/Dense>
#include <tuple>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <cmath>
#pragma warning(pop)

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// using PolyLine = Eigen::MatrixXd;
// using Point = Eigen::Vector2d;

// Eigen::VectorXd calculateIntersectionPoint(std::vector<double> line_a, std::vector<double> line_b);

double roundToXDigits(double value);
Eigen::MatrixXd cutFromPolyLine(const Eigen::MatrixXd &polyline, Eigen::VectorXd start_point, Eigen::VectorXd end_point);
Eigen::MatrixXd interpolateMatrix(const Eigen::MatrixXd &mat, int num_points);
double calculateCircumToChordDistance(double chord_length, double chord_angle);
double calculateRadiusFromChordAndCircum(double chord_length, double circum_to_chord);
Eigen::VectorXd calculateCenterFromRadiusAndChord(const Eigen::VectorXd chord_midpoint, double chord_angle, double radius, double circum_to_chord);
bool isLineIntersectedCircle(const Eigen::MatrixXd &pline, const Eigen::VectorXd &center, double R);
int getIndexOfLastNonNaN(const Eigen::VectorXd &slice_stratum, int idx = -1);

double calculateAreaOfQuad(const Eigen::VectorXd &points_x, const Eigen::VectorXd &points_y);

// std::vector<int> analysis_const;

#endif