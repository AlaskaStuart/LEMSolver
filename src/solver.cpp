#include "solver.h"

// step-1: 确定破裂面的入口和出口位置
// step-2: 插值生成入口点和出口点
// step-3: 生成滑面
// step-4: 生成切片
// step-5: 生成考虑了地层的切片
void Solver::initialize()
{
    // step-1: 确定破裂面的入口和出口位置
    Solver::entryRange_ = setEntryRange(Solver::slope_.slope_shape);
    Solver::exitRange_ = setExitRange(Solver::slope_.slope_shape);

    // step-2: 插值生成入口点和出口点
    Solver::entryPointsSet_ = interpolatedPolyLine(entryRange_,
                                                   Solver::cfg_.entry_exit_points_num);
    Solver::exitPointsSet_ = interpolatedPolyLine(exitRange_,
                                                  Solver::cfg_.entry_exit_points_num);
    // step-3: 生成滑面
    Solver::slipsSet_ = setSlips(Solver::entryPointsSet_,
                                 Solver::exitPointsSet_,
                                 Solver::cfg_.slips_num,
                                 Solver::slope_);
    // step-4: 生成切片, 圆弧型或复合型
    Solver::slicesGeomSet_ = setSlicesGeom(Solver::entryPointsSet_,
                                           Solver::exitPointsSet_,
                                           Solver::slipsSet_,
                                           Solver::slope_,
                                           Solver::cfg_.slices_num);

    // step-5: 生成考虑了地层的切片
    Solver::slicesStratumSet_ = setSlicesStratum(Solver::slicesGeomSet_,
                                                 Solver::slope_);
}
// step-6: 根据用户指定的算法计算安全系数
std::map<std::tuple<int, int, int>, float> Solver::startAnalysis(std::string methodName)
{
    // 根据methodName确定计算fs所用算法
    auto it = methodMap.find(methodName);
    if (it == methodMap.end())
        throw std::runtime_error("Unknown method: " + methodName);

    FSFunc func = it->second;

    // 初始化结果
    std::map<std::tuple<int, int, int>, float> results;

    // 所需参数: 地层, 切片
    std::vector<Strata> stratumList = Solver::slope_.stratum;
    MatrixMap slicesStratumSet = Solver::slicesStratumSet_;

    // 多线程优化
    for (auto &slicesWithStratum : slicesStratumSet)
    {
        auto slicesKey = slicesWithStratum.first;
        const auto &ithSlicesStratum = slicesWithStratum.second;

        // 计算安全系数
        float fs = func(ithSlicesStratum, stratumList);

        results[slicesKey] = fs;
    }

    return results;
}
// step-7: 返回计算结果
SolverResult Solver::buildResult() const
{
    SolverResult result;

    result.entryRange = entryRange_;
    result.exitRange = exitRange_;
    result.entryPointsSet = entryPointsSet_;
    result.exitPointsSet = exitPointsSet_;
    result.factorOfSafety = factorOfSafety_;
    result.slipsSet = slipsSet_;
    result.slicesGeomSet = slicesGeomSet_;

    return result;
}

// int main()
// {
//     // 读取json文件
//     std::ifstream f("./data/sectionModel.json");
//     if (!f.is_open())
//     {
//         std::cerr << "Error: cannot open ../../data/sectionModel.json\n";
//         return -1;
//     }
//     nlohmann::json j;

//     // 解析json文件, 得到Slope对象
//     f >> j;
//     Slope slope = j.get<Slope>();

//     // 构造LEM-Solver对象
//     Solver s = Solver(slope);
//     // 开始分析计算
//     s.startAnalysis();
//     // 获取结果
//     SolverResult result = s.getResult();

//     return 0;
// }