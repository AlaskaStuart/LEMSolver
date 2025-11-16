#include "algorithm.h"

MultiPoints2D setEntryRange(const MultiPoints2D &polyline)
{
    MultiPoints2D entryRange;

    std::array<double, 2> p1 = polyline.front();
    std::array<double, 2> p2 = polyline[1];

    entryRange.push_back(p1);
    entryRange.push_back(p2);
    double lengthP1P2 = std::sqrt(std::pow(p2[0] - p1[0], 2) + std::pow(p2[1] - p1[1], 2));
    if (lengthP1P2 > MAX_ENTRY_EXIT_LENGTH)
    {
        double x = p2[0] - MAX_ENTRY_EXIT_LENGTH;
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
    if (lengthP1P2 > MAX_ENTRY_EXIT_LENGTH)
    {
        double x = p2[0] + MAX_ENTRY_EXIT_LENGTH;
        double y = getYbyX(setExitRange, x);
        std::array<double, 2> newPoint = {x, y};
        setExitRange[1] = newPoint;
    }
}

// 每个vector的前三个元素对应的是圆弧面的
VectorMap setSlips(const MultiPoints2D &entryPointsSet,
                   const MultiPoints2D &exitPointsSet,
                   const int &numSlips,
                   const Slope &slope)
{
    VectorMap slipsMap;
    // for (const std::array<double, 2> &entryPoint : entryPointsSet)
    // {
    //     for (const std::array<double, 2> &exitPoint : exitPointsSet)
    //     {
    //         for (int i = 0; i < numSlips; i++)
    //         {
    //                     }
    //     }
    // }
    for (int i = 0; i < entryPointsSet.size(); i++)
    {
        for (int j = 0; j < exitPointsSet.size(); j++)
        {

            std::array<double, 2> entryPoint = entryPointsSet[i];
            std::array<double, 2> exitPoint = exitPointsSet[j];
            // 获得初始滑裂面圆弧
            // 以入口点和出口点的连线作为滑裂圆弧的弦
            // 获得初始的自弦至滑裂面圆弧的距离
            std::array<double, 2> ChordMid = getMidPoint(entryPoint, exitPoint);
            double chordLength = calculateLength(exitPoint, entryPoint);

            double chordHorizontalAngle = std::atan2(entryPoint[1] - exitPoint[1], entryPoint[0] - exitPoint[0]);

            double initChordtoCircum = INIT_SLIP_DEPTH_FACTOR * std::abs(2 * chordLength * std::tan(chordHorizontalAngle));

            for (int k = 0; k < numSlips; k++)
            {
                std::tuple<int, int, int> mapKey = {i, j, k};
                std::vector<double> ithArcSlip;
                // some functions

                double chordtoCircum = initChordtoCircum * (1 - k / double(numSlips));
                double radius = getRadiusByChordCircum(chordLength, chordtoCircum);
                std::array<double, 2> center = getCenter(ChordMid, chordHorizontalAngle, radius, chordtoCircum);

                // 验证是否有交点

                int intersection = intersectionNum(slope.slope_shape, center, radius);
                if (intersection < 3)
                {
                    ithArcSlip.push_back(center[0]);
                    ithArcSlip.push_back(center[1]);
                    ithArcSlip.push_back(radius);
                }

                slipsMap[mapKey] = ithArcSlip;
            }
        }
    }
    return slipsMap;
}

VectorMap setSlicesGeos(const MultiPoints2D &entryPointsSet,
                        const MultiPoints2D &exitPointsSet,
                        const VectorMap &slipsSet,
                        const Slope &slope,
                        int &slicesNum)
{

    VectorMap slicesGeos;

    for (int i = 0; i < entryPointsSet.size(); i++)
    {
        for (int j = 0; j < exitPointsSet.size(); j++)
        {
            std::array<double, 2> entryPoint = entryPointsSet[i];
            std::array<double, 2> exitPoint = exitPointsSet[j];

            std::vector<double> vectorX = interpolate(entryPoint[0], exitPoint[0], slicesNum);
            double dx = vectorX[1] - vectorX[0];
            vectorX.pop_back();
            std::for_each(vectorX.begin(), vectorX.end(), [dx](double &x)
                          { x += dx; });

            std::vector<double> vectorYtop;
            for (double midX : vectorX)
            {
                double midY = getYbyX(slope.slope_shape, midX);
                vectorYtop.push_back(midY);
            }
            // std::vector<double> vectorXMid(vectorX.size());
            // std::transform(vectorX.begin(),
            //                vectorX.end(),
            //                vectorXMid.begin(),
            //                [dx](double x)
            //                { return x + dx / 2; });
            // vectorXMid.pop_back();
            for (int k = 0; k < slipsSet.size(); k++)
            {
                std::tuple<int, int, int> mapKey = {i, j, k};

                for (double midX : vectorX)
                {
                                }
            }
        }
    }

    return slicesGeos;
}

VectorMap setSlicesStratum(const VectorMap &slipsSet,
                           const Slope &slope)
{
}