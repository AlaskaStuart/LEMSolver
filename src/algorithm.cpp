#include "algorithm.h"

// 镜像slope
std::pair<double, Slope> reverseSlope(const Slope &slope)
{
    Slope newSlope = slope;
    Point2D leftSlpPt = newSlope.slope_shape.front();
    Point2D rightSlpPt = newSlope.slope_shape.back();

    double dir = rightSlpPt[1] - leftSlpPt[1];

    // 镜像点
    double mirrorAxes = std::nan("");

    if (dir < 0)
    {
        mirrorAxes = leftSlpPt[0];

        // slope_shape
        newSlope.slope_shape = mirrorMultiPoints2DAlongX(newSlope.slope_shape, mirrorAxes);
        // stratum
        for (auto &strata : newSlope.stratum)
        {
            strata.strata_upper_bound = mirrorMultiPoints2DAlongX(strata.strata_upper_bound, mirrorAxes);
            strata.strata_lower_bound = mirrorMultiPoints2DAlongX(strata.strata_lower_bound, mirrorAxes);
        }

        // fault
        // fault不为空才进行镜像
        if (!newSlope.fault.empty())
        {
            newSlope.fault = mirrorMultiPoints2DAlongX(newSlope.fault, mirrorAxes);
        }
        // water_level
        // water_level不为空才进行镜像
        if (!newSlope.water_level.empty())
        {
            newSlope.water_level = mirrorMultiPoints2DAlongX(newSlope.water_level, mirrorAxes);
        }
    }
    return {mirrorAxes, newSlope};
}

// 设置破裂面的入口范围
// 以边坡坡顶作为入口, 且入口范围长度不超过MAX_ENTRY_EXIT_LENGTH
std::pair<MultiPoints2D, MultiPoints2D> setEntryExitRange(const MultiPoints2D &polyline)
{
    // MultiPoints2D entryRange;
    // MultiPoints2D exitRange;
    // std::pair<MultiPoints2D, MultiPoints2D> result;
    int plineSize = polyline.size();

    if (plineSize < 2)
    {
        throw std::runtime_error("Polyline size must be greater than 1.");
    }
    std::array<double, 2> p1 = polyline.front();
    std::array<double, 2> p2 = polyline[1];

    std::array<double, 2> p3 = polyline[plineSize - 2];
    std::array<double, 2> p4 = polyline.back();
    double lengthP1P2 = calculateLength(p1, p2);
    double lengthP3P4 = calculateLength(p3, p4);
    if (lengthP1P2 > MAX_ENTRY_EXIT_LENGTH)
    {
        p1[0] = p2[0] - MAX_ENTRY_EXIT_LENGTH;
        p1[1] = getYbyX(polyline, p1[0]);
    }
    if (lengthP3P4 > MAX_ENTRY_EXIT_LENGTH)
    {
        p4[0] = p3[0] + MAX_ENTRY_EXIT_LENGTH;
        p4[1] = getYbyX(polyline, p4[0]);
    }

    // 判断p1p2和p3p4那个是入口，哪个是出口
    // if (p1[1] > p3[1])
    // {
    //     entryRange.push_back(p1);
    //     entryRange.push_back(p2);
    //     exitRange.push_back(p3);
    //     exitRange.push_back(p4);
    //     result.first = entryRange;
    //     result.second = exitRange;
    // }
    // else
    // {
    //     exitRange.push_back(p1);
    //     exitRange.push_back(p2);
    //     entryRange.push_back(p3);
    //     entryRange.push_back(p4);
    //     result.first = entryRange;
    //     result.second = exitRange;
    // }
    // return result;

    // 高的那一侧作为入口, 低的那一侧作为出口
    MultiPoints2D entry, exit;
    auto pushRange = [](auto &v, const auto &a, const auto &b)
    {
        v.push_back(a);
        v.push_back(b);
    };

    if (p1[1] > p3[1])
    {
        pushRange(entry, p1, p2);
        pushRange(exit, p3, p4);
    }
    else
    {
        pushRange(exit, p1, p2);
        pushRange(entry, p3, p4);
    }
    return {entry, exit};
}

// 每个vector的前三个元素对应的是圆弧面的
// 普通圆弧滑面
// 存储的是 圆心坐标、半径
// vector长度为1
PolyLineMap setSlips(const MultiPoints2D &entryPointsSet,
                     const MultiPoints2D &exitPointsSet,
                     const int &numSlips,
                     const MultiPoints2D &slopeShape)
{
    PolyLineMap slipsMap;
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

            double initChordtoCircum = INIT_SLIP_DEPTH_FACTOR * std::abs(chordLength / 2 * std::tan(chordHorizontalAngle));

            for (int k = 0; k < numSlips; k++)
            {
                std::tuple<int, int, int> mapKey = {i, j, k};

                double chordtoCircum = initChordtoCircum * (1 - k / double(numSlips));
                double radius = getRadiusByChordCircum(chordLength, chordtoCircum);
                std::array<double, 2> center = getCenter(ChordMid, chordHorizontalAngle, radius, chordtoCircum);

                // 验证是否有交点
                // 如果圆弧面与边坡表面交点数量小于3, 则将计算结果添加到map中
                MultiPoints2D cuttedPolyline = cutFromPolyLine(slopeShape, entryPoint, exitPoint);

                int intersection = intersectionNum(cuttedPolyline, center, radius);
                // TODO: test 记得替换
                MultiPoints2D interPt = polyLineArcIntersection(cuttedPolyline, center, radius);

                if (intersection < 3)
                {
                    std::array<double, 3> ithArcGeom;

                    std::vector<std::array<double, 3>> ithArcSlip;

                    // ithArcGeom.push_back(center[0]);
                    // ithArcGeom.push_back(center[1]);
                    // ithArcGeom.push_back(radius);
                    ithArcGeom[0] = center[0];
                    ithArcGeom[1] = center[1];
                    ithArcGeom[2] = radius;

                    ithArcSlip.push_back(ithArcGeom);

                    slipsMap[mapKey] = ithArcSlip;
                }
            }
        }
    }
    return slipsMap;
}

// 如果不存在软弱层/断层，那么该方法就会退化成圆弧形滑面
// 如果返回结果长度为1，则圆弧形滑面，大于1则为非复合滑面
PolyLineMap setSlips(const MultiPoints2D &entryPointsSet,
                     const MultiPoints2D &exitPointsSet,
                     const int &numSlips,
                     const Slope &slope)
{

    PolyLineMap arcSlips = setSlips(entryPointsSet, exitPointsSet, numSlips, slope.slope_shape);

    if (slope.fault.empty())
    {
        return arcSlips;
    }

    PolyLineMap slipsMap;

    // 断层/软弱面
    MultiPoints2D weakSurface = slope.fault;
    for (auto ithArcSlipPair : arcSlips)
    {
        const std::tuple<int, int, int> &mapKey = ithArcSlipPair.first;
        const PolyLineCAD &ithArcSlip = ithArcSlipPair.second;

        int entryPtIdx = std::get<0>(mapKey);
        int exitPtIdx = std::get<1>(mapKey);
        int slipIdx = std::get<2>(mapKey);

        const std::array<double, 2> &entryPoint = entryPointsSet[entryPtIdx];
        const std::array<double, 2> &exitPoint = exitPointsSet[exitPtIdx];

        Point2D ithArcSlipCenter = {ithArcSlip[0][0], ithArcSlip[0][1]};
        double ithArcSlipRadius = ithArcSlip[0][2];

        // 这里需要保左边点的横坐标要小于右边点的横坐标'
        Point2D lPt = exitPoint;
        Point2D rPt = entryPoint;
        if (lPt[0] > rPt[0])
        {
            std::swap(lPt, rPt);
        }

        // 先检查断层、软弱层与圆弧形滑面是否有交点
        // 再检查断层、软弱层与边坡是否有交点
        // 两次检查分别有三种情景：1个交叉点，2个交叉点，无交叉点
        // 检查一有1个交点：根据下一步的判断决定情况
        // 情景1.1：断层、软弱层与边坡存在一个交点，返回复合型边坡，一半是断层、软弱层，一半是圆弧形
        // 情景1.2：断层、软弱层与边坡存在2个交点，返回为断层、软弱层
        // 情景1.3: 断层、软弱层与边坡不存在交点，返回为圆弧形滑面
        // 检查一有2个交点：返回复合型滑面，由圆弧形-断层软弱层-圆弧组成的三段式滑面
        // 情景2.2: 断层、软弱层与边坡存在两个交点，返回复合型滑面，由断层软弱层-圆弧-断层软弱层的三段式滑面
        // 情景2.1和情景2.3: 返回复合型滑面，由圆弧形-断层软弱层-圆弧组成的三段式滑面
        // 检查一无交点：根据下一步的判断决定情况
        // 情景3.1: 断层、软弱层与边坡存在一个交点，返回圆弧形滑面
        // 情景3.2: 断层、软弱层与边坡存在两个交点，判断断层软弱层与圆弧形滑面的上下关系，谁在上面，则返回哪个
        // 情景3.3: 断层、软弱层与边坡不存在交点，则返回圆弧形滑面
        // 除了以上9种情景之外的其他情景，都返回圆弧形滑面

        MultiPoints2D preWeakSurface;
        MultiPoints2D cuttedSlopeShape = cutFromPolyLine(slope.slope_shape, entryPoint, exitPoint);
        MultiPoints2D faultAndSlopeIntersection = twoPolyLineIntersection(cuttedSlopeShape, weakSurface);
        MultiPoints2D faultAndSlipIntersection = polyLineArcIntersection(weakSurface, ithArcSlipCenter, ithArcSlipRadius);
        PolyLineCAD ithSlip;

        // 情景1
        if (faultAndSlipIntersection.size() == 1)
        {
            // 情景1.1
            if (faultAndSlopeIntersection.size() == 1)
            {

                // 计算dir，边坡朝向
                double dir = entryPoint[0] - exitPoint[0];
                if (dir > 0)
                {
                    // 边坡剖面右高左低
                    preWeakSurface = cutFromPolyLine(weakSurface, faultAndSlopeIntersection[0], weakSurface[weakSurface.size() - 1]);
                }
                else
                {
                    // 边坡剖面左高右低
                    preWeakSurface = cutFromPolyLine(weakSurface, weakSurface[0], faultAndSlopeIntersection[0]);
                }

                // AB为软弱层/断层（A在左，B在右）
                Point2D vectCA = ithArcSlipCenter - preWeakSurface[0];
                Point2D vectCB = ithArcSlipCenter - preWeakSurface.back();
                // 两种情景：边坡从左至右逐渐上升、逐渐下降，A在圆弧以上、B在圆弧以下，A在圆弧以下、B在圆弧之上
                // 两种情景排列组合，共有4中情况
                // 最终归纳总结得到的结论为
                // A在圆弧之上时，先折线后圆弧
                // B在圆弧之上时，先圆弧后折线
                PolyLineCAD arcPart;
                // A在圆弧之上时
                if ((length(vectCA) < ithArcSlipRadius - EPSILON) && (length(vectCB) > ithArcSlipRadius - EPSILON))
                {
                    // TODO: 先折线段，然后圆弧段
                    //  左侧折线段
                    MultiPoints2D intermediatePart = cutFromPolyLine(preWeakSurface, lPt, faultAndSlipIntersection[0]);
                    // 去掉最后一个元素
                    intermediatePart.pop_back();
                    // 转换成PolyLineCAD
                    PolyLineCAD intermediatePartCAD = transformFromMultiPoints2D(intermediatePart);

                    // 右侧圆弧段
                    arcPart = toPolyLineCAD(faultAndSlipIntersection[0], rPt, ithArcSlipCenter, ithArcSlipRadius);

                    // 先折线段，然后圆弧段
                    ithSlip.insert(ithSlip.end(), intermediatePartCAD.begin(), intermediatePartCAD.end());
                    ithSlip.insert(ithSlip.end(), arcPart.begin(), arcPart.end());
                }
                // B在圆弧之上时
                else if ((length(vectCB) < ithArcSlipRadius - EPSILON) && (length(vectCA) > ithArcSlipRadius - EPSILON))
                {

                    // 左侧圆弧段
                    arcPart = toPolyLineCAD(lPt, faultAndSlipIntersection[0], ithArcSlipCenter, ithArcSlipRadius);
                    // 右侧折线段
                    MultiPoints2D intermediatePart = cutFromPolyLine(preWeakSurface, faultAndSlipIntersection[0], rPt);
                    // 去掉第一个元素
                    intermediatePart.erase(intermediatePart.begin());
                    // 转换成PolyLineCAD
                    PolyLineCAD intermediatePartCAD = transformFromMultiPoints2D(intermediatePart);

                    // 先圆弧段，后折线段
                    ithSlip.insert(ithSlip.end(), arcPart.begin(), arcPart.end());
                    ithSlip.insert(ithSlip.end(), intermediatePartCAD.begin(), intermediatePartCAD.end());
                }

                slipsMap[mapKey] = ithSlip;
            }
            // 情景1.2
            else if (faultAndSlopeIntersection.size() == 2)
            {
                preWeakSurface = cutFromPolyLine(weakSurface, faultAndSlopeIntersection[0], faultAndSlopeIntersection[1]);
                ithSlip = transformFromMultiPoints2D(preWeakSurface);
                slipsMap[mapKey] = ithSlip;
            }
            // 情景1.3和其他情景
            else
            {
                slipsMap[mapKey] = ithArcSlip;
            }
        }
        // 情景2
        else if (faultAndSlipIntersection.size() == 2)
        {
            // 情景2.2
            if (faultAndSlopeIntersection.size() == 2)
            {
                // 第一段多线段
                MultiPoints2D polylinePart1 = cutFromPolyLine(weakSurface, faultAndSlopeIntersection[0], faultAndSlipIntersection[0]);
                polylinePart1.pop_back();

                PolyLineCAD polylinePart1CAD = transformFromMultiPoints2D(polylinePart1);

                ithSlip.insert(ithSlip.end(), polylinePart1CAD.begin(), polylinePart1CAD.end());

                // 中间段圆弧
                PolyLineCAD arcPart = toPolyLineCAD(faultAndSlipIntersection[0], faultAndSlipIntersection[1], ithArcSlipCenter, ithArcSlipRadius);

                ithSlip.insert(ithSlip.end(), arcPart.begin(), arcPart.end());

                // 第二段多线段
                MultiPoints2D polylinePart2 = cutFromPolyLine(weakSurface, faultAndSlipIntersection[1], faultAndSlopeIntersection[1]);
                polylinePart2.erase(polylinePart2.begin());
                PolyLineCAD polylinePart2CAD = transformFromMultiPoints2D(polylinePart2);

                ithSlip.insert(ithSlip.end(), polylinePart2CAD.begin(), polylinePart2CAD.end());
            }
            // 情景2.1 情景2.3 和其他情况
            else
            {
                // 第一个交点左侧，圆弧段
                PolyLineCAD arcPart1 = toPolyLineCAD(lPt, faultAndSlipIntersection[0], ithArcSlipCenter, ithArcSlipRadius);
                ithSlip.insert(ithSlip.end(), arcPart1.begin(), arcPart1.end());

                // 两个交点中间段, 多线段
                MultiPoints2D intermediatePart = cutFromPolyLine(weakSurface, faultAndSlipIntersection[0], faultAndSlipIntersection[1]);
                // 去掉第一个元素和最后一个元素
                intermediatePart.erase(intermediatePart.begin());
                intermediatePart.pop_back();
                // 转换成PolyLineCAD
                PolyLineCAD intermediatePartCAD = transformFromMultiPoints2D(intermediatePart);
                ithSlip.insert(ithSlip.end(), intermediatePartCAD.begin(), intermediatePartCAD.end());

                // 第二个交点右侧，圆弧段
                PolyLineCAD arcPart2 = toPolyLineCAD(faultAndSlipIntersection[1], rPt, ithArcSlipCenter, ithArcSlipRadius);

                ithSlip.insert(ithSlip.end(), arcPart2.begin(), arcPart2.end());
            }

            slipsMap[mapKey] = ithSlip;
        }
        // 情景3
        else if (faultAndSlipIntersection.empty())
        {
            // 情景3.2
            if (faultAndSlopeIntersection.size() == 2)
            {
                preWeakSurface = cutFromPolyLine(weakSurface, faultAndSlopeIntersection[0], faultAndSlopeIntersection[1]);

                // AB为软弱层/断层（A在左，B在右）
                Point2D vectCA = ithArcSlipCenter - preWeakSurface[0];
                Point2D vectCB = ithArcSlipCenter - preWeakSurface.back();

                if ((length(vectCA) < ithArcSlipRadius - EPSILON) && (length(vectCB) < ithArcSlipRadius - EPSILON))
                {
                    ithSlip = transformFromMultiPoints2D(preWeakSurface);
                    slipsMap[mapKey] = ithSlip;
                }
                else
                {
                    slipsMap[mapKey] = ithArcSlip;
                }
            }
            // 情景3.1 情景3.3 和其他情况
            else
            {
                slipsMap[mapKey] = ithArcSlip;
            }
        }
        else
        {
            // 除了上述9种情景之外的其他情景：返回圆弧滑面
            slipsMap[mapKey] = ithArcSlip;
        }
    }
    // 去除重复值
    PolyLineMap dedupedSlipsMap;
    std::set<PolyLineCAD> seenSlips;
    for (auto &slipPair : slipsMap)
    {
        if (seenSlips.insert(slipPair.second).second)
        {
            dedupedSlipsMap.emplace(slipPair.first, slipPair.second);
        }
    }
    return dedupedSlipsMap;
}

// 每个索引的值为该{入口点, 出口点, 滑裂面}组合所对应的分割条带
// 分割条带是个二维数组, shape = (3,slicesNum)
// 第一行: 每个条带对应的x值
// 第二行: 每个条带对应的y_top值
// 第三行: 每个条带对应的y_bot值
MatrixMap setSlicesGeom(const MultiPoints2D &entryPointsSet,
                        const MultiPoints2D &exitPointsSet,
                        const PolyLineMap &slipsSet,
                        const MultiPoints2D &slopeShape,
                        int &slicesNum)
{

    MatrixMap results;

    // 插值生成条带, 不包含入口点和出口点, 个数为slicesNum
    // 因此取参数时slicesNum + 2
    // 生成slices的时候移除最后一个
    // 返回的结果是slicesNum+1个长度
    // 最后在计算安全系数时, 只考虑前slicesNum个条带,实际利用上的条带数量为sliceNum个
    int matrixRow = slicesNum + 1;
    for (auto &slip : slipsSet)
    {
        // 减少深拷贝，提升效率
        const std::tuple<int, int, int> &mapKey = slip.first;
        const PolyLineCAD &ithslip = slip.second;
        // const std::vector<std::array<double, 3>> &ithslip = slip.second;

        CustomMatrix ithMat(3, matrixRow);

        int entryPtIdx = std::get<0>(mapKey);
        int exitPtIdx = std::get<1>(mapKey);
        int slipIdx = std::get<2>(mapKey);

        const std::array<double, 2> &entryPoint = entryPointsSet[entryPtIdx];
        const std::array<double, 2> &exitPoint = exitPointsSet[exitPtIdx];

        // 切片的x值
        std::vector<double> vectorX;
        // 切片的yTop值
        std::vector<double> vectorYtop;
        // 切片的yBot值
        std::vector<double> vectorYbot;

        // 圆弧滑面计算方式
        if (ithslip.size() == 1)
        {
            // 查看是否与坡型方向有关
            // 插值生成x
            // 入口点和出口点的x值进行插值
            vectorX = interpolate(entryPoint[0], exitPoint[0], matrixRow + 1);
            double dx = vectorX[1] - vectorX[0];

            // 移除最后一个元素
            vectorX.pop_back();
            // x取切条的中值
            std::for_each(vectorX.begin(), vectorX.end(), [dx](double &x)
                          { x += dx / 2; });

            // yTop
            // 根据x在slopeShape上找到相应的y值
            for (double midX : vectorX)
            {
                double midY = getYbyX(slopeShape, midX);
                vectorYtop.push_back(midY);
            }
            // yBot
            // 根据x与滑面圆之间的关系生成y值
            // | [x, yBot]-[xC, yC] | = R
            // xC, yC, R已知, 带入x可求得yBot
            // 圆弧的圆心坐标和半径
            std::array<double, 3> ithSlipCircle = ithslip.at(0);

            double slipCX = std::get<0>(ithSlipCircle);
            double slipCY = std::get<1>(ithSlipCircle);
            double slipCR = std::get<2>(ithSlipCircle);

            // std::vector<double> vectorYbot(vectorX.size());
            vectorYbot.reserve(vectorX.size());
            std::transform(vectorX.begin(),
                           vectorX.end(),
                           vectorYbot.begin(),
                           [slipCX, slipCY, slipCR](double x)
                           {
                               return slipCY - std::sqrt(slipCR * slipCR - (slipCX - x) * (slipCX - x));
                           });
        }
        // 复合滑面计算方式
        else
        {
            // 确保上一个方法返回的切片为已经和圆弧滑面、边坡坡型相切之后的结果。
            std::array<double, 3> slipLeft = ithslip.at(0);
            std::array<double, 3> slipRight = ithslip.at(ithslip.size() - 1);

            // x由画面的起始点和终止点插值生成，从大到小
            // 取ithslip第一个值和最后一个值，由第一个值和最后一个值插值生成x
            // 需要起始点和终止点
            double slipLeftX = std::get<0>(slipLeft);
            double slipRightX = std::get<0>(slipRight);
            vectorX = interpolate(slipLeftX, slipRightX, matrixRow + 1);
            double dx = vectorX[1] - vectorX[0];
            // 移除最后一个元素
            vectorX.pop_back();
            // x取切条的中值
            std::for_each(vectorX.begin(), vectorX.end(), [dx](double &x)
                          { x += dx / 2; });

            for (double ithX : vectorX)
            {
                // 切片的yBot值
                // 遍历ithslip，判断每个点的第三个元素是否为0，判断的时候考虑到误差
                // 若为0，则使用getYbyX()函数计算y值
                // 若不为0，半径，圆心坐标，计算y值,复制上面的计算方式
                for (int slipPtIdx = 0; slipPtIdx < ithslip.size() - 1; slipPtIdx++)
                {
                    // 非圆弧面，正常插值
                    if (std::abs(ithslip.at(slipPtIdx)[2]) < EPSILON)
                    {

                        MultiPoints2D ithPart;
                        ithPart.reserve(2);
                        Point2D prevPt = {ithslip.at(slipPtIdx)[0], ithslip.at(slipPtIdx)[1]};
                        Point2D nextPt = {ithslip.at(slipPtIdx + 1)[0], ithslip.at(slipPtIdx + 1)[1]};
                        ithPart.push_back(prevPt);
                        ithPart.push_back(nextPt);
                        // PolyLineCAD ithPart = {ithslip.at(slipPtIdx), ithslip.at(slipPtIdx + 1)};

                        double ithY = getYbyX(ithPart, ithX);
                        if (!std::isnan(ithY))
                        {
                            vectorYbot.push_back(ithY);
                            break;
                        }
                    }
                    // 圆弧面，按照圆弧坐标公式插值
                    else
                    {

                        PolyLineCAD ithPart;
                        ithPart.reserve(2);
                        std::array<double, 3> prevPt = ithslip.at(slipPtIdx);
                        std::array<double, 3> nextPt = ithslip.at(slipPtIdx + 1);
                        ithPart.push_back(prevPt);
                        ithPart.push_back(nextPt);

                        // 由多线段计算出圆心和半径
                        // Point2D ithPartCenter;
                        // double radius;
                        // ithPartCenter = std::get<2>(fromPolyLineCAD(ithPart));
                        // radius = std::get<3>(fromPolyLineCAD(ithPart));

                        auto [A, B, ithPartCenter, radius, ccw] = fromPolyLineCAD(ithPart);

                        double ithY = ithPartCenter[1] - std::sqrt(radius * radius - (ithPartCenter[0] - ithX) * (ithPartCenter[0] - ithX));
                        vectorYbot.push_back(ithY);
                        break;
                    }
                }

                // 切片的yTop值
                double ithYtop = getYbyX(slopeShape, ithX);
                if (!std::isnan(ithYtop))
                    vectorYtop.push_back(ithYtop);
            }
        }
        // 保存ithMat
        ithMat.setRow(0, vectorX);
        ithMat.setRow(1, vectorYtop);
        ithMat.setRow(2, vectorYbot);

        results[mapKey] = ithMat;
    }

    return results;
}

// 值为CustomMatrix
// CustomMatrix第0行为x值
// CustomMatrix第1+2*i行为第i层顶面的切片
// CustomMatrix第1+2*i+1行为第i层底面的切片
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

CalcuInitArgs getInitArgs(const CustomMatrix &slicesStratum, std::vector<Strata> &stratum)
{

    int stratumSize = stratum.size();
    std::vector<double> slicesX = slicesStratum.getRow(0);

    // 条带宽度
    double dx = slicesX[1] - slicesX[0];

    // 边坡方向: 左低右高为正，左高右低为负.
    // 找到最左边条带的顶点y值, 即入口点y值
    // 找到最右边条带的顶点y值, 即出口点y值
    const std::vector<double> &firstSlice = slicesStratum.getCol(0);
    const std::vector<double> &lastSlice = slicesStratum.getCol(slicesStratum.getRow(0).size() - 1);
    int leftIdx = 1;
    while (leftIdx < firstSlice.size() && (std::isnan(firstSlice[leftIdx])))
    {
        leftIdx += 1;
    }
    double beginning = firstSlice[leftIdx];

    int rightIdx = 1;
    while (rightIdx < lastSlice.size() && (std::isnan(lastSlice[rightIdx])))
    {
        rightIdx++;
    }
    double ending = lastSlice[rightIdx];
    // 最右边的y值-最左边的y值,  两者的x之差只能为正值
    // 因此y值之差的正负决定边坡方向
    double dir = ending - beginning;

    return {stratumSize, slicesX, dx, dir};
}

// 瑞典条分法
double calculateFSOrdinary(const CustomMatrix &slicesStratum, std::vector<Strata> &stratum)
{

    double pushingForce = 0.0;
    double resistanceForce = 0.0;

    auto [stratumSize, slicesX, dx, dir] = getInitArgs(slicesStratum, stratum);

    // 沿x轴自左至右
    // 逐个条带计算重量以及条带底部的摩擦力和黏聚力。
    // 为防止out_of_range error, 最后一个条带不纳入计算
    for (int sliceIdx = 0; sliceIdx < slicesX.size() - 1; sliceIdx++)
    {
        // 条带的滑移力和抗滑移力
        double sliceResistanceForce = 0;
        double slicePushForce = 0;

        const std::vector<double> &currentSlice = slicesStratum.getCol(sliceIdx);
        const std::vector<double> &nextSlice = slicesStratum.getCol(sliceIdx + 1);

        // 找到条带底部所属的地层
        int currSliceBotIdx = getSliceBotIndex(currentSlice);
        int nextSliceBotIdx = getSliceBotIndex(nextSlice);

        // 获取切片底部所在地层的黏聚力和内摩擦角
        double botStrataCohesion = stratum[currSliceBotIdx / 2 - 1].cohesion;
        double botStrataFrictionAngle = stratum[currSliceBotIdx / 2 - 1].friction_angle;

        // 计算条带底边的长度和倾角
        double botDy = nextSlice[nextSliceBotIdx] - currentSlice[currSliceBotIdx];

        double botLength = sqrt(botDy * botDy + dx * dx);
        double botAngle = atan(botDy / dx);
        // 计算条带的黏聚力、摩擦力和重力

        // 1.黏聚力
        double sliceCohesionForce = botStrataCohesion * botLength;

        // 计算条块重量
        // weight = sum(dx*dy*density)
        double sliceWeight = 0.0;
        for (int strataIdx = 0; strataIdx < stratumSize; strataIdx++)
        {
            double currSliceStrataTop = currentSlice[1 + strataIdx * 2];
            double currSliceStrataBot = currentSlice[1 + strataIdx * 2 + 1];

            if (std::isnan(currSliceStrataTop) || std::isnan(currSliceStrataBot))
            {
                continue;
            }
            else
            {
                double density = stratum[strataIdx].density;
                double dy = currSliceStrataTop - currSliceStrataBot;

                sliceWeight += dy * density * dx;
            }
        }
        // 摩擦力
        double sliceFriction = abs(sliceWeight * cos(botAngle) * tan(botStrataFrictionAngle * PI / 180));

        // 若条带底部倾角与边坡倾向方向相反，则可确定为滑面前端倒角
        // 摩擦力方向相反, 无推动力, 所有力参与抵抗
        if (botAngle * dir < 0)
        {
            // 条带的抵抗力 = 条带黏聚力 + 摩擦力 + 重力分量
            sliceResistanceForce = sliceCohesionForce + sliceFriction + abs(sliceWeight * sin(botAngle));
        }
        // 摩擦力方向正常
        else
        {
            // 条带的抵抗力 = 条带黏聚力 + 摩擦力
            sliceResistanceForce = sliceCohesionForce + sliceFriction;

            // 条带的推动力 = 条带重力在条带底部倾角方向上的分力
            slicePushForce = abs(sliceWeight * sin(botAngle));
        }

        resistanceForce += sliceResistanceForce;
        pushingForce += slicePushForce;
    }

    double result = resistanceForce / pushingForce;

    return result;
}

// 毕晓普法
double calculateFSBishop(const CustomMatrix &slicesStratum, std::vector<Strata> &stratum)
{
    double prevFS = calculateFSOrdinary(slicesStratum, stratum);

    auto [stratumSize, slicesX, dx, dir] = getInitArgs(slicesStratum, stratum);

    for (int iteration = 0; iteration < INTERATION_NUM; iteration++)
    {
        double pushingForce = 0.0;
        double resistanceForce = 0.0;

        for (int sliceIdx = 0; sliceIdx < slicesX.size() - 1; sliceIdx++)
        {

            // 条带的滑移力和抗滑移力
            double sliceResistanceForce = 0;
            double slicePushForce = 0;

            const std::vector<double> &currentSlice = slicesStratum.getCol(sliceIdx);
            const std::vector<double> &nextSlice = slicesStratum.getCol(sliceIdx + 1);

            // 找到条带底部所属的地层
            int currSliceBotIdx = getSliceBotIndex(currentSlice);
            int nextSliceBotIdx = getSliceBotIndex(nextSlice);

            // 获取切片底部所在地层的黏聚力和内摩擦角
            double botStrataCohesion = stratum[currSliceBotIdx / 2 - 1].cohesion;
            double botStrataFrictionAngle = stratum[currSliceBotIdx / 2 - 1].friction_angle;

            // 计算条带底边的长度和倾角
            double botDy = nextSlice[nextSliceBotIdx] - currentSlice[currSliceBotIdx];

            // double botLength = std::sqrt(botDy * botDy + dx * dx);
            double botAngle = atan(botDy / dx);

            // 计算条带的黏聚力、摩擦力和重力

            // 1. 黏聚力
            double sliceCohesionForce = botStrataCohesion * dx;

            // 2. 计算条块重量
            // weight = sum(dx*dy*density)
            double sliceWeight = 0.0;
            for (int strataIdx = 0; strataIdx < stratumSize; strataIdx++)
            {
                double currSliceStrataTop = currentSlice[1 + strataIdx * 2];
                double currSliceStrataBot = currentSlice[1 + strataIdx * 2 + 1];

                if (std::isnan(currSliceStrataTop) || std::isnan(currSliceStrataBot))
                {
                    continue;
                }
                else
                {
                    double density = stratum[strataIdx].density;
                    double dy = currSliceStrataTop - currSliceStrataBot;

                    sliceWeight += dy * density * dx;
                }
            }

            // 平衡方程中的抗滑力系数
            double fa = abs(cos(botAngle)) + sin(botAngle) * tan(botStrataFrictionAngle * PI / 180) / prevFS;

            // 3. 摩擦力
            double sliceFriction = abs(sliceWeight * tan(botStrataFrictionAngle * PI / 180));

            // 若条带底部倾角与边坡倾向方向相反，则可确定为滑面前端倒角
            // 摩擦力方向相反, 无推动力, 所有力参与抵抗
            if (botAngle * dir < 0)
            {
                // 条带的抵抗力 = 条带黏聚力 + 摩擦力 + 重力分量
                // TODO: 这里该返回0还是继续
                sliceResistanceForce = (sliceCohesionForce + sliceFriction + abs(sliceWeight * sin(botAngle))) / fa;
            }
            // 摩擦力方向正常
            else
            {
                // 条带的抵抗力 = 条带黏聚力 + 摩擦力
                sliceResistanceForce = (sliceCohesionForce + sliceFriction) / fa;

                // 条带的推动力 = 条带重力在条带底部倾角方向上的分力
                slicePushForce = abs(sliceWeight * sin(botAngle));
            }

            resistanceForce += sliceResistanceForce;
            pushingForce += slicePushForce;
        }

        double currFS = resistanceForce / pushingForce;
        if (std::abs(currFS - prevFS) < EPSILON)
        {
            prevFS = currFS;
            break;
        }
        else
        {
            prevFS = currFS;
        }
    }
    return prevFS;
}

// 简布法
double calculateFSJanbu(const CustomMatrix &slicesStratum, std::vector<Strata> &stratum)
{
    double prevFS = calculateFSOrdinary(slicesStratum, stratum);

    int stratumSize = stratum.size();
    const std::vector<double> &slicesX = slicesStratum.getRow(0);

    // 条带宽度
    double dx = slicesX[1] - slicesX[0];

    // 边坡方向: 左低右高为正，左高右低为负.
    // 找到最左边条带的顶点y值, 即入口点y值
    // 找到最右边条带的顶点y值, 即出口点y值
    const std::vector<double> &firstSlice = slicesStratum.getCol(0);
    const std::vector<double> &lastSlice = slicesStratum.getCol(slicesStratum.getRow(0).size() - 1);
    int leftIdx = 1;
    while (leftIdx < firstSlice.size() && (std::isnan(firstSlice[leftIdx])))
    {
        leftIdx += 1;
    }
    double beginning = firstSlice[leftIdx];

    int rightIdx = 1;
    while (rightIdx < lastSlice.size() && (std::isnan(lastSlice[rightIdx])))
    {
        rightIdx++;
    }
    double ending = lastSlice[rightIdx];
    // 最右边的y值-最左边的y值,  两者的x之差只能为正值
    // 因此y值之差的正负决定边坡方向
    double dir = ending - beginning;

    double dirDx = slicesX[-1] - slicesX[0];

    std::array<double, 2> entryPoint = {slicesX[0], beginning};
    std::array<double, 2> exitPoint = {slicesX[-1], ending};

    double chordLen = calculateLength(entryPoint, exitPoint);
    double maxChordToSlip = 0;

    // 计算f0系数
    double correctionB1 = 0.5;
    double correctionF0 = 1;
    for (int sliceIdx = 0; sliceIdx < slicesX.size() - 1; sliceIdx++)
    {

        const std::vector<double> &currentSlice = slicesStratum.getCol(sliceIdx);
        const std::vector<double> &nextSlice = slicesStratum.getCol(sliceIdx + 1);

        // 找到条带底部所属的地层
        int currSliceBotIdx = getSliceBotIndex(currentSlice);
        int nextSliceBotIdx = getSliceBotIndex(nextSlice);

        // 计算条带底边的长度和倾角
        double botDy = nextSlice[nextSliceBotIdx] - currentSlice[currSliceBotIdx];

        double botY = (nextSlice[nextSliceBotIdx] + currentSlice[currSliceBotIdx]) / 2;
        Point2D ptBot = {slicesX[sliceIdx] + dx / 2, botY};

        double chordToSlip = calculateDistToPolyline(ptBot, entryPoint, exitPoint).second;

        maxChordToSlip = std::max(maxChordToSlip, chordToSlip);

        double correctionDratioL = maxChordToSlip / chordLen;
        correctionF0 = 1 + correctionB1 * (correctionDratioL - 1.4 * correctionDratioL * correctionDratioL);
    }

    for (int iteration = 0; iteration < INTERATION_NUM; iteration++)
    {
        double pushingForce = 0.0;
        double resistanceForce = 0.0;

        for (int sliceIdx = 0; sliceIdx < slicesX.size() - 1; sliceIdx++)
        {

            // 条带的滑移力和抗滑移力
            double sliceResistanceForce = 0;
            double slicePushForce = 0;

            const std::vector<double> &currentSlice = slicesStratum.getCol(sliceIdx);
            const std::vector<double> &nextSlice = slicesStratum.getCol(sliceIdx + 1);

            // 找到条带底部所属的地层
            int currSliceBotIdx = getSliceBotIndex(currentSlice);
            int nextSliceBotIdx = getSliceBotIndex(nextSlice);

            // 获取切片底部所在地层的黏聚力和内摩擦角
            double botStrataCohesion = stratum[currSliceBotIdx / 2 - 1].cohesion;
            double botStrataFrictionAngle = stratum[currSliceBotIdx / 2 - 1].friction_angle;

            // 计算条带底边的长度和倾角
            double botDy = nextSlice[nextSliceBotIdx] - currentSlice[currSliceBotIdx];

            // double botLength = std::sqrt(botDy * botDy + dx * dx);
            double botAngle = atan(botDy / dx);

            // 计算条带的黏聚力、摩擦力和重力

            // 1. 黏聚力
            double sliceCohesionForce = botStrataCohesion * dx;

            // 2. 计算条块重量
            // weight = sum(dx*dy*density)
            double sliceWeight = 0.0;
            for (int strataIdx = 0; strataIdx < stratumSize; strataIdx++)
            {
                double currSliceStrataTop = currentSlice[1 + strataIdx * 2];
                double currSliceStrataBot = currentSlice[1 + strataIdx * 2 + 1];

                if (std::isnan(currSliceStrataTop) || std::isnan(currSliceStrataBot))
                {
                    continue;
                }
                else
                {
                    double density = stratum[strataIdx].density;
                    double dy = currSliceStrataTop - currSliceStrataBot;

                    sliceWeight += dy * density * dx;
                }
            }

            // 3. 摩擦力
            double ma = abs(std::cos(botAngle) * (1 + 1 / prevFS * std::tan(botStrataFrictionAngle * PI / 180) * std::tan(botAngle)));
            double sliceFriction = abs(tan(botStrataFrictionAngle * PI / 180) * (sliceWeight - abs(sin(botAngle) * sliceCohesionForce) / prevFS) / ma);

            // 若条带底部倾角与边坡倾向方向相反，则可确定为滑面前端倒角
            // 摩擦力方向相反, 无推动力, 所有力参与抵抗
            if (botAngle * dir < 0)
            {
                // 条带的抵抗力 = 条带黏聚力 + 摩擦力 + 重力分量
                // TODO: 这里该返回0还是继续
                sliceResistanceForce = sliceCohesionForce + sliceFriction + abs(sliceWeight * std::tan(botAngle));
            }
            // 摩擦力方向正常
            else
            {
                // 条带的抵抗力 = 条带黏聚力 + 摩擦力
                sliceResistanceForce = sliceCohesionForce + sliceFriction;

                // 条带的推动力 = 条带重力在条带底部倾角方向上的分力
                slicePushForce = abs(sliceWeight * tan(botAngle));
            }

            resistanceForce += sliceResistanceForce;
            pushingForce += slicePushForce;
        }

        double currFS = correctionF0 * resistanceForce / pushingForce;
        if (std::abs(currFS - prevFS) < EPSILON)
        {
            prevFS = currFS;
            break;
        }
        else
        {
            prevFS = currFS;
        }
    }
    return prevFS;
}

// double calculateFSSpencer(const CustomMatrix &slicesStratum, std::vector<Strata> &stratum)
// {
//     float result = -1.0;
//     float pushingForce = 0.0;

//     return result;
// }

// double calculateFSMorgensternPrice(const CustomMatrix &slicesStratum, std::vector<Strata> &stratum)
// {
//     float result = -1.0;
//     float pushingForce = 0.0;

//     return result;
// }

// std::map<std::string, FSFunc> methodMap = {
//     {"Ordinary", calculateFSOrdinary},
//     {"Bishop", calculateFSBishop},
//     {"Janbu", calculateFSJanbu},
//     {"Spencer", calculateFSSpencer},
//     {"MorgensternPrice", calculateFSMorgensternPrice}};

std::map<std::string, FSFunc> methodMap = {
    {"Ordinary", calculateFSOrdinary},
    {"Bishop", calculateFSBishop},
    {"Janbu", calculateFSJanbu}};