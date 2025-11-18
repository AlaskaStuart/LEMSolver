// 从json文件中读取界面数据，生成Slope结构体

#ifndef LEMSOLVER_MODEL_H
#define LEMSOLVER_MODEL_H

#include "utils.h"

struct Strata
{

    std::string strata_name;
    int strata_idx;
    float density;
    float cohesion;
    float friction_angle;
    MultiPoints2D strata_upper_bound;
    MultiPoints2D strata_lower_bound;
};

struct Fault
{

    int fault_id;
    MultiPoints2D fault_shape;
};
struct Slope
{

    MultiPoints2D slope_shape;
    std::vector<struct Strata> stratum;
    std::vector<struct Fault> faults;
    MultiPoints2D water_level;
};

void from_json(const json &j, Slope &s);
void from_json(const json &j, Strata &s);
void from_json(const json &j, Fault &s);

#endif
