#ifndef LEMSOLVER_ALGORITHM_H
#define LEMSOLVER_ALGORITHM_H

#include "model.h"
#include "geometry.h"

using FSFunc = double (*)(const CustomMatrix &slicesStratum, std::vector<Strata> &stratum);

std::map<std::string, FSFunc> methodMap = {
    {"Ordinary", calculateFSOrdinary},
    {"Bishop", calculateFSBishop},
    {"Janbu", calculateFSJanbu},
    {"Spencer", calculateFSSpencer},
    {"MorgensternPrice", calculateFSMorgensternPrice}};

MultiPoints2D setEntryRange(const MultiPoints2D &polyline);
MultiPoints2D setExitRange(const MultiPoints2D &polyline);

VectorMap setSlips(const MultiPoints2D &entryPointsSet,
                   const MultiPoints2D &exitPointsSet,
                   const int &numSlips,
                   const Slope &slope);

MatrixMap setSlicesGeom(const MultiPoints2D &entryPointsSet,
                        const MultiPoints2D &exitPointsSet,
                        const VectorMap &slipsSet,
                        const Slope &slope,
                        int &slicesNum);

MatrixMap setSlicesStratum(const MatrixMap &slicesMap,
                           const Slope &slope);

double calculateFSOrdinary(const CustomMatrix &slicesStratum, std::vector<Strata> &stratum);

double calculateFSBishop(const CustomMatrix &slicesStratum, std::vector<Strata> &stratum);

double calculateFSJanbu(const CustomMatrix &slicesStratum, std::vector<Strata> &stratum);

double calculateFSSpencer(const CustomMatrix &slicesStratum, std::vector<Strata> &stratum);

double calculateFSMorgensternPrice(const CustomMatrix &slicesStratum, std::vector<Strata> &stratum);

#endif