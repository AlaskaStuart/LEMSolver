#include "model.h"
#include <iostream>
#include <fstream>
using json = nlohmann::json;


// ---------- 工具函数：JSON数组 → Eigen矩阵 ----------
static Eigen::MatrixX2d json_to_matrix2d(const json& arr) {
    Eigen::MatrixX2d mat(arr.size(), 2);
    for (int i = 0; i < arr.size(); ++i) {
        mat(i, 0) = arr[i][0].get<double>();
        mat(i, 1) = arr[i][1].get<double>();
    }
    return mat;
}

// ---------- from_json 定义 ----------
void from_json(const json& j, Strata& s) {
    j.at("name").get_to(s.strata_name);
    j.at("idx").get_to(s.strata_id);
    j.at("density").get_to(s.density);
    j.at("cohesion").get_to(s.cohension);
    j.at("phi").get_to(s.friction_angle);

    s.stratum_upper_bound = json_to_matrix2d(j.at("upperBound"));
    s.stratum_lower_bound = json_to_matrix2d(j.at("lowerBound"));

}

void from_json(const json& j, Fault& f) {

    if(j.contains("fault_id")){
        j.at("fault_id").get_to(f.fault_id);
    }
    if (j.contains("fault_shape"))
        f.fault_shape = json_to_matrix2d(j.at("fault_shape"));
}

void from_json(const json& j, Slope& s) {

    s.slope_shape = json_to_matrix2d(j.at("slopeShape"));

    // 如果无水位，初始化为空
    s.water_level = Eigen::MatrixX2d(); 
    if (j.contains("waterLevel") && !j["waterLevel"].empty())
        s.water_level = json_to_matrix2d(j["waterLevel"]);

    j.at("stratum").get_to(s.stratum);
    if (j.contains("faults"))
        j.at("faults").get_to(s.faults);
}

int main() {
    std::ifstream f("../data/sectionModel.json");
    nlohmann::json j;
    f >> j;

    Slope slope = j.get<Slope>();

    std::cout << "Slope points: " << slope.slope_shape.rows() << "\n";
    std::cout << "Strata count: " << slope.stratum.size() << "\n";
    std::cout << "First stratum name: " << slope.stratum[0].strata_name << "\n";
}