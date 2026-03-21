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

// 根据情况反转结果
void Solver::mirrorResult()
{
    // entryRange
    this->entryRange_ = mirrorMultiPoints2DAlongX(this->entryRange_, this->mirrorX_);
    this->exitRange_ = mirrorMultiPoints2DAlongX(this->exitRange_, this->mirrorX_);

    this->entryPointsSet_ = mirrorMultiPoints2DAlongX(this->entryPointsSet_, this->mirrorX_);
    this->exitPointsSet_ = mirrorMultiPoints2DAlongX(this->exitPointsSet_, this->mirrorX_);

    PolyLineMap newSlipsSet;
    // slicesGeomSet_只更新第一行
    MatrixMap newSlicesGeomSet;

    std::map<std::tuple<int, int, int>, double> newFactorOfSafety;
    for (auto &slip : this->slipsSet_)
    {
        // 生成新的mapkey
        std::tuple<int, int, int> originMapKey = slip.first;
        int newEntryIdx = this->cfg_.entry_exit_points_num - 1 - std::get<0>(originMapKey);
        int newExitIdx = this->cfg_.entry_exit_points_num - 1 - std::get<1>(originMapKey);
        std::tuple<int, int, int> newMapKey = std::make_tuple(newEntryIdx, newExitIdx, std::get<2>(originMapKey));

        // 镜像slip
        PolyLineCAD originSlip = slip.second;
        for (auto &pt : originSlip)
        {
            pt[0] = -pt[0] + 2 * this->mirrorX_;
        }
        std::reverse(originSlip.begin(), originSlip.end());
        if (originSlip.size() > 1)
        {
            for (int ptIdx = 1; ptIdx < originSlip.size(); ptIdx++)
            {
                double curveVal = originSlip[ptIdx][2];
                if (curveVal != 0)
                {
                    // 将曲率值赋给上一行
                    originSlip[ptIdx - 1][2] = curveVal;
                    originSlip[ptIdx][2] = 0;
                }
            }
        }
        // 镜像slices
        CustomMatrix ithSlices = this->slicesGeomSet_[originMapKey];
        for (int i = 0; i < ithSlices.cols(); i++)
        {
            ithSlices(0, i) = -ithSlices(0, i) + 2 * this->mirrorX_;
        }
        newSlipsSet[newMapKey] = originSlip;
        newSlicesGeomSet[newMapKey] = ithSlices;
        newFactorOfSafety[newMapKey] = this->factorOfSafety_[originMapKey];
    }
    // 更新slips
    this->slipsSet_ = newSlipsSet;
    // 更新slicesGeomSet
    this->slicesGeomSet_ = newSlicesGeomSet;
    // 更新factorOfSafety的mapkey
    this->factorOfSafety_ = newFactorOfSafety;
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

    // 如果左高右低: 镜像结果
    if (!std::isnan(this->mirrorX_))
    {
        mirrorResult();
    }
}
