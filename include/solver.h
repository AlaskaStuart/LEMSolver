#ifndef LEMSOLVER_SLOVER_H
#define LEMSOLVER_SLOVER_H

// #include "utils.h"
#include "model.h"
#include "algorithm.h"

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
    std::map<std::tuple<int, int, int>, float> factorOfSafety;
    PolyLineMap slipsSet;
    MatrixMap slicesGeomSet;
};

class Solver
{
public:
    explicit Solver(const Slope &slope, const SolverConfig &cfg = SolverConfig())
        : slope_(slope), cfg_(cfg)
    {
        initialize();
    };

    // 方便画图
    const MultiPoints2D &getEntryRange() const { return entryRange_; };
    const MultiPoints2D &getExitRange() const { return exitRange_; };
    const MultiPoints2D &getEntryPointsSet() const { return entryPointsSet_; };
    const MultiPoints2D &getExitPointsSet() const { return exitPointsSet_; };
    // CustomMatrix getArcSlipsSet() const { return slipsSet_; };
    // CustomMatrix getSlicesGeosSet() const { return slicesGeosSet_; };
    // CustomMatrix getSlicesStratumSet() const { return slicesStratumSet_; };

    void startAnalysis(std::string methodName = "Ordinary");
    const SolverResult getResult() const { return buildResult(); };

private:
    SolverConfig cfg_;
    Slope slope_;

    MultiPoints2D entryRange_;
    MultiPoints2D exitRange_;

    MultiPoints2D entryPointsSet_;
    MultiPoints2D exitPointsSet_;

    PolyLineMap slipsSet_;
    MatrixMap slicesGeomSet_;
    MatrixMap slicesStratumSet_;

    std::map<std::tuple<int, int, int>, float> factorOfSafety_;

    void initialize();

    SolverResult buildResult() const;
};

#endif