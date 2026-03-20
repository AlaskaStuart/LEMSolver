#ifndef LEMSOLVER_ALGORITHM_H
#define LEMSOLVER_ALGORITHM_H

#include "model.h"
#include "geometry.h"
#include "settings.h"

using FSFunc = double (*)(const CustomMatrix &slicesStratum, std::vector<Strata> &stratum);

struct CalcuInitArgs
{
    // 地层数量
    int stratumSize;

    // 条带的x坐标
    std::vector<double> slicesX;

    // 条带宽度
    double dx;

    // 判断边坡坡型方向
    double dir;
};

std::pair<double, Slope> reverseSlope(const Slope &slope);

std::pair<MultiPoints2D, MultiPoints2D> setEntryExitRange(const MultiPoints2D &polyline);

// 圆弧滑面
PolyLineMap setSlips(const MultiPoints2D &entryPointsSet,
                     const MultiPoints2D &exitPointsSet,
                     const int &numSlips,
                     const MultiPoints2D &slopeShape);

// 复合滑面
PolyLineMap setSlips(const MultiPoints2D &entryPointsSet,
                     const MultiPoints2D &exitPointsSet,
                     const int &numSlips,
                     const Slope &slope);

// MatrixMap setSlicesGeom(const MultiPoints2D &entryPointsSet,
//                         const MultiPoints2D &exitPointsSet,
//                         const VectorMap &slipsSet,
//                         const Slope &slope,
//                         int &slicesNum);
MatrixMap setSlicesGeom(const MultiPoints2D &entryPointsSet,
                        const MultiPoints2D &exitPointsSet,
                        const PolyLineMap &slipsSet,
                        const MultiPoints2D &slopeShape,
                        int &slicesNum);

MatrixMap setSlicesStratum(const MatrixMap &slicesMap,
                           const Slope &slope);

CalcuInitArgs getInitArgs(const CustomMatrix &slicesStratum, std::vector<Strata> &stratum);

double calculateFSOrdinary(const CustomMatrix &slicesStratum, std::vector<Strata> &stratum);

double calculateFSBishop(const CustomMatrix &slicesStratum, std::vector<Strata> &stratum);

double calculateFSJanbu(const CustomMatrix &slicesStratum, std::vector<Strata> &stratum);

// double calculateFSSpencer(const CustomMatrix &slicesStratum, std::vector<Strata> &stratum);

// double calculateFSMorgensternPrice(const CustomMatrix &slicesStratum, std::vector<Strata> &stratum);

extern std::map<std::string, FSFunc> methodMap;

#endif