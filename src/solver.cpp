#include "solver.h"

// step-1: 确定破裂面的入口和出口位置
// step-2: 插值生成入口点和出口点
// step-3: 生成滑面
// step-4: 生成切片
// step-5: 生成考虑了地层的切片
void Solver::initialize()
{
    // 先判断边坡形态是左高右低还是右高左低
    // 如果是左高右低，镜像翻转slope
    std::pair<double, Slope> flippedSlope = reverseSlope(this->slope_);
    this->mirrorX_ = flippedSlope.first;
    this->mediaSlope_ = flippedSlope.second;

    // step-1: 确定破裂面的入口和出口位置
    std::pair<MultiPoints2D, MultiPoints2D> entryExit = setEntryExitRange(this->mediaSlope_.slope_shape);
    this->entryRange_ = entryExit.first;
    this->exitRange_ = entryExit.second;

    // step-2: 插值生成入口点和出口点
    this->entryPointsSet_ = interpolatedPolyLine(this->entryRange_,
                                                 this->cfg_.entry_exit_points_num);
    this->exitPointsSet_ = interpolatedPolyLine(this->exitRange_,
                                                this->cfg_.entry_exit_points_num);

    // step-3: 生成滑面, 圆弧型或复合型
    // 目前做法默认是复合型滑面，向下兼容圆弧形滑面
    // 如果存在软弱层/断层，先简单预处理，超出预处理的预设条件之外的其他情景，则退化为圆弧型滑面
    this->slipsSet_ = setSlips(this->entryPointsSet_,
                               this->exitPointsSet_,
                               this->cfg_.slips_num,
                               this->mediaSlope_);
    // step-4: 生成切片
    this->slicesGeomSet_ = setSlicesGeom(this->entryPointsSet_,
                                         this->exitPointsSet_,
                                         this->slipsSet_,
                                         this->mediaSlope_.slope_shape,
                                         this->cfg_.slices_num);

    // step-5: 生成考虑了地层的切片
    this->slicesStratumSet_ = setSlicesStratum(this->slicesGeomSet_,
                                               this->mediaSlope_);
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
    std::vector<Strata> stratumList = this->mediaSlope_.stratum;
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
SolverResult Solver::buildResult()
{
    SolverResult result;

    // 如果mirrorX不是nan, 则反转结果
    if (!std::isnan(this->mirrorX_))
    {
        // 如果之前镜像过, 这里再次镜像
        // entryRange
        this->entryRange_ = mirrorMultiPoints2DAlongX(this->entryRange_, this->mirrorX_);
        this->exitRange_ = mirrorMultiPoints2DAlongX(this->exitRange_, this->mirrorX_);

        this->entryPointsSet_ = mirrorMultiPoints2DAlongX(this->entryPointsSet_, this->mirrorX_);
        this->exitPointsSet_ = mirrorMultiPoints2DAlongX(this->exitPointsSet_, this->mirrorX_);

        // TODO
        // 生成新的mapKey
        // 镜像多线段
        // 第一列镜像
        // 第二列不变
        // slips第三列如果不为空, 找到不为零的行, 将该值赋给上一行, 然后本行变为0
    }

    result.entryRange = this->entryRange_;
    result.exitRange = this->exitRange_;
    result.entryPointsSet = this->entryPointsSet_;
    result.exitPointsSet = this->exitPointsSet_;
    result.factorOfSafety = this->factorOfSafety_;
    result.slipsSet = this->slipsSet_;
    result.slicesGeomSet = this->slicesGeomSet_;

    return result;
}
