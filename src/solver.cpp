#include "solver.h"

// step-1: 确定破裂面的入口和出口位置
// step-2: 插值生成入口点和出口点
// step-3: 生成滑面
// step-4: 生成切片
// step-5: 生成考虑了地层的切片
void Solver::initialize()
{
    // step-1: 确定破裂面的入口和出口位置
    std::pair<MultiPoints2D, MultiPoints2D> entryExit = setEntryExitRange(this->slope_.slope_shape);
    this->entryRange_ = entryExit.first;
    this->exitRange_ = entryExit.second;

    // step-2: 插值生成入口点和出口点
    this->entryPointsSet_ = interpolatedPolyLine(this->entryRange_,
                                                 this->cfg_.entry_exit_points_num);
    this->exitPointsSet_ = interpolatedPolyLine(this->exitRange_,
                                                this->cfg_.entry_exit_points_num);

    // step-3: 生成滑面, 圆弧型或复合型
    // TODO: 加个判断，是否采用复合滑面
    // 还是说要暴露接口，让用户自行判断是否选择复合型滑面
    this->slipsSet_ = setSlips(this->entryPointsSet_,
                               this->exitPointsSet_,
                               this->cfg_.slips_num,
                               this->slope_.slope_shape);
    // step-4: 生成切片
    this->slicesGeomSet_ = setSlicesGeom(this->entryPointsSet_,
                                         this->exitPointsSet_,
                                         this->slipsSet_,
                                         this->slope_.slope_shape,
                                         this->cfg_.slices_num);

    // step-5: 生成考虑了地层的切片
    this->slicesStratumSet_ = setSlicesStratum(this->slicesGeomSet_,
                                               this->slope_);
}

// step-6: 根据用户指定的算法计算安全系数
void Solver::startAnalysis(std::string methodName)
{
    // 根据methodName确定计算fs所用算法
    auto it = methodMap.find(methodName);
    if (it == methodMap.end())
        throw std::runtime_error("Unknown method: " + methodName);

    FSFunc func = it->second;

    // 初始化结果
    std::map<std::tuple<int, int, int>, double> results;

    // 所需参数: 地层, 切片
    std::vector<Strata> stratumList = this->slope_.stratum;
    MatrixMap slicesStratumSet = this->slicesStratumSet_;

    // TODO: 多线程优化
    for (auto &slicesWithStratum : slicesStratumSet)
    {
        auto slicesKey = slicesWithStratum.first;
        const auto &ithSlicesStratum = slicesWithStratum.second;

        // 计算安全系数
        double fs = func(ithSlicesStratum, stratumList);

        results[slicesKey] = fs;
    }

    this->factorOfSafety_ = results;
}

// step-7: 返回计算结果
SolverResult Solver::buildResult() const
{
    SolverResult result;

    result.entryRange = this->entryRange_;
    result.exitRange = this->exitRange_;
    result.entryPointsSet = this->entryPointsSet_;
    result.exitPointsSet = this->exitPointsSet_;
    result.factorOfSafety = this->factorOfSafety_;
    result.slipsSet = this->slipsSet_;
    result.slicesGeomSet = this->slicesGeomSet_;

    return result;
}
