#ifndef LEMSOLVER_SLOVER_H
#define LEMSOLVER_SLOVER_H

#include "model.h"
#include "algorithm.h"
#include "settings.h"

/*
args:
- cfg: 计算配置文件
- slope: 斜坡模型

methods:
- startAnalysis(methodName)
- getResult()

output:
- factorOfSafety: 系数安全系数
-
*/

// 保存极限平衡法计算结果
struct SolverResult
{
    // plot elements
    MultiPoints2D entryRange;
    MultiPoints2D exitRange;
    MultiPoints2D entryPointsSet;
    MultiPoints2D exitPointsSet;
    PolyLineMap slipsSet;
    MatrixMap slicesGeomSet;
    std::map<std::tuple<int, int, int>, double> factorOfSafety;
};

class Solver
{
public:
    explicit Solver(const Slope &slope, const SolverConfig &cfg = SolverConfig())
        : slope_(slope), cfg_(cfg)
    {
        initialize();
    };

    void startAnalysis(std::string methodName = "Ordinary");

    const MultiPoints2D &getEntryRange() const { return entryRange_; };
    const MultiPoints2D &getExitRange() const { return exitRange_; };
    const MultiPoints2D &getEntryPointsSet() const { return entryPointsSet_; };
    const MultiPoints2D &getExitPointsSet() const { return exitPointsSet_; };
    SolverResult getResult() const
    {
        return SolverResult{
            entryRange_,
            exitRange_,
            entryPointsSet_,
            exitPointsSet_,
            slipsSet_,
            slicesGeomSet_,
            factorOfSafety_,
        };
    };

private:
    SolverConfig cfg_;
    Slope slope_;

    // 镜像对称轴
    double mirrorX_;
    // 计算使用
    Slope mediaSlope_;

    MultiPoints2D entryRange_;
    MultiPoints2D exitRange_;

    MultiPoints2D entryPointsSet_;
    MultiPoints2D exitPointsSet_;

    PolyLineMap slipsSet_;
    MatrixMap slicesGeomSet_;
    MatrixMap slicesStratumSet_;

    std::map<std::tuple<int, int, int>, double> factorOfSafety_;

    void initialize();
    void mirrorResult();
};

#endif