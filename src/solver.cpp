#include "solver.h"

MultiPoints2D setEntryRange(const MultiPoints2D &polyline)
{
}

MultiPoints2D setExitRange(const MultiPoints2D &polyline)
{
}

DynamicContainer setSlips(const MultiPoints2D &entryPointsSet,
                          const MultiPoints2D &exitPointsSet,
                          const Slope &slope)
{
}

DynamicContainer setSlicesGeos(const MultiPoints2D &entryPointsSet,
                               const MultiPoints2D &exitPointsSet,
                               const DynamicContainer &slipsSet,
                               const Slope &slope)
{
}

DynamicContainer setSlicesStratum(const DynamicContainer &slipsSet,
                                  const Slope &slope)
{
}
// step-1: 确定破裂面的入口和出口位置
// step-2:
void Solver::init()
{
    Solver::entryRange_ = setEntryRange(Solver::slope_.slope_shape);
    Solver::exitRange_ = setExitRange(Solver::slope_.slope_shape);

    Solver::entryPointsSet_ = interpolatedPolyLine(entryRange_,
                                                   Solver::cfg_.entry_exit_points_num);
    Solver::exitPointsSet_ = interpolatedPolyLine(exitRange_,
                                                  Solver::cfg_.entry_exit_points_num);

    Solver::slipsSet_ = setSlips(Solver::entryPointsSet_,
                                 Solver::exitPointsSet_,
                                 Solver::slope_);

    Solver::slicesGeosSet_ = setSlicesGeos(Solver::entryPointsSet_,
                                           Solver::exitPointsSet_,
                                           Solver::slipsSet_,
                                           Solver::slope_);

    Solver::slicesStratumSet_ = setSlicesStratum(Solver::slipsSet_,
                                                 Solver::slope_);
}

std::vector<float> Solver::startAnalysis(std::string method)
{
}

SolverResult Solver::buildResult() const
{
    SolverResult result;

    result.entryRange = entryRange_;
    result.exitRange = exitRange_;
    result.entryPointsSet = entryPointsSet_;
    result.exitPointsSet = exitPointsSet_;
    result.factorOfSafety = factorOfSafety_;
    result.arcSlipsSet = slipsSet_;
    result.slicesGeosSet = slicesGeosSet_;

    return result;
}