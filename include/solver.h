#ifndef LEMSOLVER_SLOVER_H
#define LEMSOLVER_SLOVER_H

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
class Solver
{
public:
    Solver(const Slope &slope, const SolverConfig &cfg = SolverConfig())
        : slope_(slope), cfg_(cfg)
    {
        initialize();
    };

    // 方便画图
    const MultiPoints2D &getEntryRange() const { return entryRange_; };
    const MultiPoints2D &getExitRange() const { return exitRange_; };
    const MultiPoints2D &getEntryPointsSet() const { return entryPointsSet_; };
    const MultiPoints2D &getExitPointsSet() const { return exitPointsSet_; };
    // DynamicContainer getArcSlipsSet() const { return slipsSet_; };
    // DynamicContainer getSlicesGeosSet() const { return slicesGeosSet_; };
    // DynamicContainer getSlicesStratumSet() const { return slicesStratumSet_; };

    void startAnalysis(std::string method = "ordinary");
    const SolverResult &getResult() const { return buildResult(); };

private:
    SolverConfig cfg_;
    Slope slope_;

    MultiPoints2D entryRange_;
    MultiPoints2D exitRange_;

    MultiPoints2D entryPointsSet_;
    MultiPoints2D exitPointsSet_;

    VectorMap slipsSet_;
    VectorMap slicesGeosSet_;
    VectorMap slicesStratumSet_;

    std::vector<float> factorOfSafety_;

    // SolverResult result_;

    void initialize();

    SolverResult buildResult() const;
};

#endif