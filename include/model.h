#ifndef LEMSOLVER_MODEL_H
#define LEMSOLVER_MODEL_H

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <Eigen/Dense>
#include <nlohmann/json.hpp>

struct Slope{

    Eigen::MatrixX2d slope_shape;
    std::vector<struct Strata> stratum;
    std::vector<struct Fault> faults;
    Eigen::MatrixX2d water_level;
};

struct Strata{

    std::string strata_name;
    int strata_id;
    float cohension;
    float friction_angle;
    float density;
    Eigen::MatrixX2d stratum_upper_bound;
    Eigen::MatrixX2d stratum_lower_bound;
};

struct Fault{

    int fault_id;
    Eigen::MatrixX2d fault_shape;
};



void from_json(const nlohmann::json& j, Slope& s);
void from_json(const nlohmann::json& j, Strata& s);
void from_json(const nlohmann::json& j, Fault& s);

#endif
