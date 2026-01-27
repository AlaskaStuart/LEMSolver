#include "algorithm.h"

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
                // TODO: 将center,radius, entryPoint, exitPoint聚合成PolyLinCAD
                int intersection = intersectionNum(cuttedPolyline, center, radius);
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

// TODO: 复合滑面
// 存储的是CAD版本的多线段
// vector长度不唯一
PolyLineMap setSlips(const MultiPoints2D &entryPointsSet,
                     const MultiPoints2D &exitPointsSet,
                     const int &numSlips,
                     const Slope &slope)
{

    PolyLineMap arcSlips = setSlips(entryPointsSet, exitPointsSet, numSlips, slope.slope_shape);

    if (slope.faults.empty())
    {
        return arcSlips;
    }

    // TODO:
    // 软弱层和断层、圆弧滑面、边坡表面
    // 需要
    // 先计算软弱层/断层与圆弧滑面之间是否存在交点，交点个数是多少
    // 如果存在交点，现在只考虑两种情况，一个交点的情况，两个交点的情况
    // 如果交点只有一个，那么判断软弱层/断层哪一端距离圆心点更近
    // 更近的那一端与交点之间的折线段保留，交点与剩余的那部分圆弧面加入到折线段

    // 如果交点存在两个，左右端点为圆弧段，交点范围内为折线段
    PolyLineMap slipsMap;
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

        // 检查圆弧是否与软弱层/断层相交
        // 需要的参数有
        // 1. 圆弧的圆心坐标
        // 2. 圆弧半径
        // 3. 起止点、终止点
        // 4. 软弱层/断层
        // TODO: fault struct重写
        PolyLineCAD ithCompositeSlip = generateCompositeSlip(entryPoint, exitPoint, ithArcSlipCenter, ithArcSlipRadius, slope.faults.front().fault_shape);
        slipsMap[mapKey] = ithCompositeSlip;
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
            // TODO: 需要上一个方法返回的切片为已经和圆弧滑面、边坡坡型相切之后的结果。
            std::array<double, 3> slipBegin = ithslip.at(0);
            std::array<double, 3> slipEnd = ithslip.at(ithslip.size() - 1);

            // x由画面的起始点和终止点插值生成，从大到小
            // 取ithslip第一个值和最后一个值，由第一个值和最后一个值插值生成x
            // 需要起始点和终止点
            double slipBeginX = std::get<0>(slipBegin);
            double slipEndX = std::get<0>(slipEnd);
            vectorX = interpolate(slipBeginX, slipEndX, matrixRow + 1);
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
                for (int slipPtIdx = 0; slipPtIdx < ithslip.size() - 2; slipPtIdx++)
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

                        double ithY = getYbyX(ithPart, ithX);
                        if (!isnan(ithY))
                        {
                            vectorYbot.push_back(ithY);
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
                        Point2D ithPartCenter;
                        double radius;

                        ithPartCenter = std::get<2>(polyLineCADToPointAndCircle(ithPart));
                        radius = std::get<3>(polyLineCADToPointAndCircle(ithPart));

                        double ithY = ithPartCenter[1] - std::sqrt(radius * radius - (ithPartCenter[0] - ithX) * (ithPartCenter[0] - ithX));
                        vectorYbot.push_back(ithY);
                    }
                }

                // 切片的yTop值
                double ithYtop = getYbyX(slopeShape, ithX);
                if (!isnan(ithYtop))
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

    return {stratumSize, slicesX, dx, dir};
}

double calculateFSOrdinary(const CustomMatrix &slicesStratum, std::vector<Strata> &stratum)
{

    double pushingForce = 0.0;
    double resistanceForce = 0.0;

    auto [stratumSize, slicesX, dx, dir] = getInitArgs(slicesStratum, stratum);

    // 沿x轴自左至右
    // 逐个条带计算重量以及条带底部的摩擦力和黏聚力。
    // 为防止out_of_range error, 最后一个条带不纳入计算
    for (int sliceIdx = 0; sliceIdx < slicesX.size() - 2; sliceIdx++)
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

        double botLength = std::sqrt(botDy * botDy + dx * dx);
        double botAngle = std::atan2(botDy, dx);

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
        double sliceFriction = abs(sliceWeight * std::cos(botAngle) * std::tan(botStrataFrictionAngle));

        // 若条带底部倾角与边坡倾向方向相反，则可确定为滑面前端倒角
        // 摩擦力方向相反, 无推动力, 所有力参与抵抗
        if (botAngle * dir < 0)
        {
            // 条带的抵抗力 = 条带黏聚力 + 摩擦力 + 重力分量
            sliceResistanceForce = sliceCohesionForce + sliceFriction + abs(sliceWeight * std::sin(botAngle));
        }
        // 摩擦力方向正常
        else
        {
            // 条带的抵抗力 = 条带黏聚力 + 摩擦力
            sliceResistanceForce = sliceCohesionForce + sliceFriction;

            // 条带的推动力 = 条带重力在条带底部倾角方向上的分力
            slicePushForce = sliceWeight * std::sin(botAngle);
        }

        resistanceForce += sliceResistanceForce;
        pushingForce += slicePushForce;
    }

    double result = resistanceForce / pushingForce;

    return result;
}

double calculateFSBishop(const CustomMatrix &slicesStratum, std::vector<Strata> &stratum)
{
    double prevFS = calculateFSOrdinary(slicesStratum, stratum);

    auto [stratumSize, slicesX, dx, dir] = getInitArgs(slicesStratum, stratum);

    for (int iteration = 0; iteration < INTERATION_NUM; iteration++)
    {
        double pushingForce = 0.0;
        double resistanceForce = 0.0;

        for (int sliceIdx = 0; sliceIdx < slicesX.size() - 2; sliceIdx++)
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
            double botAngle = std::atan2(botDy, dx);

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
            double fa = abs(std::cos(botAngle)) + sin(botAngle) * std::tan(botStrataFrictionAngle) / prevFS;

            // 3. 摩擦力
            double sliceFriction = sliceWeight * std::tan(botStrataFrictionAngle);

            // 若条带底部倾角与边坡倾向方向相反，则可确定为滑面前端倒角
            // 摩擦力方向相反, 无推动力, 所有力参与抵抗
            if (botAngle * dir < 0)
            {
                // 条带的抵抗力 = 条带黏聚力 + 摩擦力 + 重力分量
                // TODO: 这里该返回0还是继续
                sliceResistanceForce = (sliceCohesionForce + sliceFriction + abs(sliceWeight * std::sin(botAngle))) / fa;
            }
            // 摩擦力方向正常
            else
            {
                // 条带的抵抗力 = 条带黏聚力 + 摩擦力
                sliceResistanceForce = (sliceCohesionForce + sliceFriction) / fa;

                // 条带的推动力 = 条带重力在条带底部倾角方向上的分力
                slicePushForce = sliceWeight * std::sin(botAngle);
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

    for (int iteration = 0; iteration < INTERATION_NUM; iteration++)
    {
        double pushingForce = 0.0;
        double resistanceForce = 0.0;

        for (int sliceIdx = 0; sliceIdx < slicesX.size() - 2; sliceIdx++)
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

            double botY = (nextSlice[nextSliceBotIdx] + currentSlice[currSliceBotIdx]) / 2;
            Point2D ptBot = {slicesX[sliceIdx] + dx / 2, botY};

            double chordToSlip = calculateDistToPolyline(ptBot, entryPoint, exitPoint).second;

            maxChordToSlip = std::max(maxChordToSlip, chordToSlip);

            // double botLength = std::sqrt(botDy * botDy + dx * dx);
            double botAngle = std::atan2(botDy, dx);

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
            double fa = (std::cos(botAngle) * std::cos(botAngle) * (1 + std::tan(botAngle) * std::tan(botStrataFrictionAngle))) / prevFS;

            // 3. 摩擦力
            double sliceFriction = sliceWeight * std::tan(botStrataFrictionAngle);

            // 若条带底部倾角与边坡倾向方向相反，则可确定为滑面前端倒角
            // 摩擦力方向相反, 无推动力, 所有力参与抵抗
            if (botAngle * dir < 0)
            {
                // 条带的抵抗力 = 条带黏聚力 + 摩擦力 + 重力分量
                // TODO: 这里该返回0还是继续
                sliceResistanceForce = (sliceCohesionForce + sliceFriction + abs(sliceWeight * std::sin(botAngle))) / fa;
            }
            // 摩擦力方向正常
            else
            {
                // 条带的抵抗力 = 条带黏聚力 + 摩擦力
                sliceResistanceForce = (sliceCohesionForce + sliceFriction) / fa;

                // 条带的推动力 = 条带重力在条带底部倾角方向上的分力
                slicePushForce = sliceWeight * std::tan(botAngle);
            }

            resistanceForce += sliceResistanceForce;
            pushingForce += slicePushForce;
        }

        double correctionB1 = 0.5;
        double correctionDratioL = maxChordToSlip / chordLen;
        double correctionF0 = 1 + correctionB1 * (correctionDratioL - 1.4 * correctionDratioL * correctionDratioL);

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