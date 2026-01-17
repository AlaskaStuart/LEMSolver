#include "model.h"

// ---------- 工具函数：JSON数组 → vector数组 ----------
static MultiPoints2D json_to_points(const json &arr)
{
    MultiPoints2D points;
    points.reserve(arr.size());

    for (const auto &item : arr)
    {
        points.push_back({item[0].get<double>(), item[1].get<double>()});
    }
    // sort the polyline by x-axis from small to large
    if (points.front()[0] > points.back()[0])
    {
        std::reverse(points.begin(), points.end());
    };
    return points;
}

// ---------- from_json 定义 ----------
void from_json(const json &j, Strata &s)
{
    j.at("strata_name").get_to(s.strata_name);
    j.at("strata_idx").get_to(s.strata_idx);
    j.at("density").get_to(s.density);
    j.at("cohesion").get_to(s.cohesion);
    j.at("friction_angle").get_to(s.friction_angle);

    s.strata_upper_bound = json_to_points(j.at("strata_upper_bound"));
    s.strata_lower_bound = json_to_points(j.at("strata_lower_bound"));
}

void from_json(const json &j, Fault &f)
{

    if (j.contains("fault_id"))
    {
        j.at("fault_id").get_to(f.fault_id);
    }
    if (j.contains("fault_shape"))
        f.fault_shape = json_to_points(j.at("fault_shape"));
}

void from_json(const json &j, Slope &s)
{

    s.slope_shape = json_to_points(j.at("slope_shape"));

    // 如果无水位，初始化为空
    s.water_level.clear();
    if (j.contains("water_level") && !j["water_level"].empty())
        s.water_level = json_to_points(j["water_level"]);

    j.at("stratum").get_to(s.stratum);

    if (j.contains("faults"))
        j.at("faults").get_to(s.faults);
}

Slope readSlopeFromJson(const std::string &filename)
{

    std::cout << "Current path: " << std::filesystem::current_path() << std::endl;

    std::ifstream f(filename);
    if (!f.is_open())
    {
        throw std::runtime_error("cannot open file: " + filename);
    }
    nlohmann::json j;
    f >> j;
    Slope slope = j.get<Slope>();
    return slope;
}