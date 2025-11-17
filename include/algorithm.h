#ifndef LEMSOLVER_ALGORITHM_H
#define LEMSOLVER_ALGORITHM_H

#include "model.h"
#include "geometry.h"

MultiPoints2D setEntryRange(const MultiPoints2D &polyline);
MultiPoints2D setExitRange(const MultiPoints2D &polyline);

VectorMap setSlips(const MultiPoints2D &entryPointsSet,
                   const MultiPoints2D &exitPointsSet,
                   const int &numSlips,
                   const Slope &slope);

// double calculateLength(std::array<double, 2Ui64> &exitPoint, std::array<double, 2Ui64> &entryPoint);

// const std::array<double, 2Ui64> &getMidPoint(std::array<double, 2Ui64> &entryPoint, std::array<double, 2Ui64> &exitPoint);

MatrixMap setSlicesGeom(const MultiPoints2D &entryPointsSet,
                        const MultiPoints2D &exitPointsSet,
                        const VectorMap &slipsSet,
                        const Slope &slope,
                        int &slicesNum);

MatrixMap setSlicesStratum(const MatrixMap &slicesMap,
                           const Slope &slope);

#endif