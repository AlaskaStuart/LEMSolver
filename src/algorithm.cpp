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

// 每个索引的值为该{入口点, 出口点, 滑裂面}组合所对应的分割条带
// 分割条带是个二维数组, shape = (3,slicesNum)
// 第一行: 每个条带对应的x值
// 第二行: 每个条带对应的y_top值
// 第三行: 每个条带对应的y_bot值
MatrixMap setSlicesGeom(const MultiPoints2D &entryPointsSet,
                        const MultiPoints2D &exitPointsSet,
                        const VectorMap &slipsSet,
                        const Slope &slope,
                        int &slicesNum)
{

    MatrixMap results;
    for (auto &slip : slipsSet)
    {
        std::tuple<int, int, int> mapKey = slip.first;
        std::vector<double> ithslip = slip.second;

        CustomMatrix ithMat(3, slicesNum);

        int entryPtIdx = std::get<0>(mapKey);
        int exitPtIdx = std::get<1>(mapKey);
        int slipIdx = std::get<2>(mapKey);

        std::array<double, 2> entryPoint = entryPointsSet[entryPtIdx];
        std::array<double, 2> exitPoint = exitPointsSet[exitPtIdx];

        // x取切条的中值
        std::vector<double> vectorX = interpolate(entryPoint[0], exitPoint[0], slicesNum + 1);
        double dx = vectorX[1] - vectorX[0];
        vectorX.pop_back();
        std::for_each(vectorX.begin(), vectorX.end(), [dx](double &x)
                      { x += dx; });

        // yTop
        std::vector<double> vectorYtop;
        for (double midX : vectorX)
        {
            double midY = getYbyX(slope.slope_shape, midX);
            vectorYtop.push_back(midY);
        }

        // yBot
        // 圆弧的圆心坐标和半径
        double slipCX = ithslip[0];
        double slipCY = ithslip[1];
        double slipCR = ithslip[2];

        std::vector<double> vectorYbot(vectorX.size());
        std::transform(vectorX.begin(),
                       vectorX.end(),
                       vectorYbot.begin(),
                       [slipCX, slipCY, slipCR](double x)
                       {
                           return slipCY - std::sqrt(slipCR * slipCR - (slipCX - x) * (slipCX - x));
                       });

        // 保存ithMat
        ithMat.setRow(0, vectorX);
        ithMat.setRow(1, vectorYtop);
        ithMat.setRow(2, vectorYbot);

        results[mapKey] = ithMat;
    }

    return results;
}

MatrixMap setSlicesStratum(const MatrixMap &slicesMap,
                           const Slope &slope)
{
    MatrixMap results;

    std::vector<struct Strata> stratums = slope.stratum;
    int stratumSize = stratums.size();

    for (auto &slices : slicesMap)
    {
        std::tuple<int, int, int> mapKey = slices.first;
        CustomMatrix ithSlicesGeom = slices.second;

        std::vector<double> slicesX = ithSlicesGeom.getRow(0);
        std::vector<double> slicesYtop = ithSlicesGeom.getRow(1);
        std::vector<double> slicesYbot = ithSlicesGeom.getRow(2);

        // 初始化ithSlicesStratum
        CustomMatrix ithSlicesStratum(1 + 2 * stratumSize, slicesX.size());
        ithSlicesStratum.setRow(0, slicesX);

        for (int i = 0; i < stratumSize; i++)
        {
            Strata ithStrata = stratums[i];

            MultiPoints2D ithUpperBound = ithStrata.strata_upper_bound;
            MultiPoints2D ithLowerBound = ithStrata.strata_lower_bound;

            // 如果第i个条带与地层顶面和底面的交点在边坡表面之上, 或者顶面和底面的交点在滑坡面之下，
            // 代表该地层与第i个条带不相交, 数值设为nan
            // 如果第i个条带与地层顶面和底面的交点在边坡表面和滑裂面之间
            // 代表第i个条带与地层完全相交
            // 其他两种情况:
            // 顶面交点在边坡表面之上, 底面交点在边坡表面和滑裂面之间, 取边坡表面点作为顶面交点
            // 顶面交点在边坡表面和滑裂面之间, 底面交点在滑裂面之下, 取滑裂面点作为底面交点
            std::vector<double> ithStrataTopVect;
            std::vector<double> ithStrataBotVect;

            for (int sliceIdx = 0; sliceIdx < slicesX.size(); sliceIdx++)
            {
                double ithSliceX = slicesX[sliceIdx];
                double ithSliceYtop = slicesYtop[sliceIdx];
                double ithSliceYbot = slicesYbot[sliceIdx];

                double ithSliceStrataYtop = getYbyX(ithUpperBound, ithSliceX);
                double ithSliceStrataYbot = getYbyX(ithLowerBound, ithSliceX);
                // 底面交点在边坡表面之上, 或顶面交点在滑裂面之下
                // 设为空
                if ((ithSliceYbot > ithSliceStrataYtop) || (ithSliceYtop < ithSliceStrataYbot))
                {
                    ithStrataTopVect.push_back(std::nan(""));
                    ithStrataBotVect.push_back(std::nan(""));
                }
                // // 顶面和底面的交点在边坡表面和滑裂面之间
                // // 正常保存
                // else if ((ithSliceYtop > ithSliceStrataYtop) && (ithSliceYbot < ithSliceStrataYbot))
                // {
                //     ithStrataTopVect.push_back(ithSliceYtop);

                //     ithStrataBotVect.push_back(ithSliceYbot);
                // }
                // // 顶面交点在边坡表面之上, 底面交点在边坡表面和滑裂面之间
                // // 底面交点在滑裂面之下, 顶面交点在边坡表面和滑裂面之间
                // else
                // {
                //     double topVal = std::min(ithSliceYtop, ithSliceStrataYtop);
                //     double botVal = std::max(ithSliceYbot, ithSliceStrataYbot);
                //     ithStrataTopVect.push_back(topVal);
                //     ithStrataBotVect.push_back(botVal);
                // }
                else
                {
                    double topVal = std::min(ithSliceYtop, ithSliceStrataYtop);
                    double botVal = std::max(ithSliceYbot, ithSliceStrataYbot);
                    ithStrataTopVect.push_back(topVal);
                    ithStrataBotVect.push_back(botVal);
                }
            }

            // 将地层相交情况保存至 ithSlicesStratum
            ithSlicesStratum.setRow(1 + 2 * i, ithStrataTopVect);
            ithSlicesStratum.setRow(1 + 2 * i + 1, ithStrataBotVect);
        }

        results[mapKey] = ithSlicesStratum;
    }

    return results;
}