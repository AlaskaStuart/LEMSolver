#ifndef LEMSOLVER_ALGORITHM_H
#define LEMSOLVER_ALGORITHM_H

#include "model.h"
#include "geometry.h"

MultiPoints2D setEntryRange(const MultiPoints2D &polyline);
MultiPoints2D setExitRange(const MultiPoints2D &polyline);

DynamicContainer setSlips(const MultiPoints2D &entryPointsSet,
                          const MultiPoints2D &exitPointsSet,
                          const int &numSlips,
                          const Slope &slope);

DynamicContainer setSlicesGeos(const MultiPoints2D &entryPointsSet,
                               const MultiPoints2D &exitPointsSet,
                               const DynamicContainer &slipsSet,
                               const Slope &slope);

DynamicContainer setSlicesStratum(const DynamicContainer &slipsSet,
                                  const Slope &slope);

#endif