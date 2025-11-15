#include "algorithm.h"

MultiPoints2D setEntryRange(const MultiPoints2D &polyline)
{
    MultiPoints2D entryRange;
    std::array<double, 2> p1 = polyline.front();
    std::array<double, 2> p2 = polyline[1];
    entryRange.push_back(p1);
    entryRange.push_back(p2);
    double lengthP1P2 = std::sqrt(std::pow(p2[0] - p1[0], 2) + std::pow(p2[1] - p1[1], 2));
    if (lengthP1P2 > 200)
    {
        double x = p2[0] - 200;
        double y = getYbyX(entryRange, x);
        std::array<double, 2> newPoint = {x, y};
        entryRange[0] = newPoint;
    }
    return entryRange;
}

MultiPoints2D setExitRange(const MultiPoints2D &polyline)
{
    MultiPoints2D setExitRange;
    std::array<double, 2> p1 = polyline.back();
    std::array<double, 2> p2 = polyline[polyline.size() - 2];
    setExitRange.push_back(p1);
    setExitRange.push_back(p2);
    double lengthP1P2 = std::sqrt(std::pow(p2[0] - p1[0], 2) + std::pow(p2[1] - p1[1], 2));
    if (lengthP1P2 > 200)
    {
        double x = p2[0] + 200;
        double y = getYbyX(setExitRange, x);
        std::array<double, 2> newPoint = {x, y};
        setExitRange[1] = newPoint;
    }
}

DynamicContainer setSlips(const MultiPoints2D &entryPointsSet,
                          const MultiPoints2D &exitPointsSet,
                          const int &numSlips,
                          const Slope &slope)
{
    DynamicContainer slipsSet(numSlips, 5);

    return slipsSet;
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