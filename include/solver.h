#ifndef LEMSOLVER_SLOVER_H
#define LEMSOLVER_SLOVER_H

#include "algorithm.h"

/*
class Solver

*/
class Solver
{
public:
    Solver(const SolverConfig &cfg = {},
           const Slope &slope,
           std::string methodName = "Ordinary")
        : cfg_(cfg), slope_(slope)
    {
        init();

        factorOfSafety_ = startAnalysis(methodName);
    };

    // 方便画图
    const MultiPoints2D &getEntryRange() const { return entryRange_; };
    const MultiPoints2D &getExitRange() const { return exitRange_; };
    const MultiPoints2D &getEntryPointsSet() const { return entryPointsSet_; };
    const MultiPoints2D &getExitPointsSet() const { return exitPointsSet_; };
    // DynamicContainer getArcSlipsSet() const { return slipsSet_; };
    // DynamicContainer getSlicesGeosSet() const { return slicesGeosSet_; };
    // DynamicContainer getSlicesStratumSet() const { return slicesStratumSet_; };
    const SolverResult &getResult() const { return buildResult(); };

private:
    SolverConfig cfg_;
    Slope slope_;

    MultiPoints2D entryRange_;
    MultiPoints2D exitRange_;

    MultiPoints2D entryPointsSet_;
    MultiPoints2D exitPointsSet_;

    DynamicContainer slipsSet_;
    DynamicContainer slicesGeosSet_;
    DynamicContainer slicesStratumSet_;

    std::vector<float> factorOfSafety_;

    // SolverResult result_;

    // 初始化函数
    void init();
    std::vector<float> startAnalysis(std::string method);

    SolverResult buildResult() const;
};

#endif