#include "solver.h"

int main()
{
    // TEST: model 的读取功能
    //  std::cout << "Current path: " << std::filesystem::current_path() << "\n";
    //  std::ifstream f("./data/sectionModel.json");
    //  if (!f.is_open())
    //  {
    //      std::cerr << "Error: cannot open ../../data/sectionModel.json\n";
    //      return -1;
    //  }
    //  nlohmann::json j;
    //  f >> j;
    //  Slope slope = j.get<Slope>();

    // std::cout << "Slope points: " << slope.slope_shape.size() << "\n";
    // std::cout << "Strata count: " << slope.stratum.size() << "\n";
    // std::cout << "First stratum name: " << slope.stratum[0].strata_name << "\n";

    // // TEST:geometry.h的getMidPoint, calculateLength
    // std::array<double, 2> p1 = {0, 0};
    // std::array<double, 2> p2 = {1, 1};
    // std::array<double, 2> midPoint = getMidPoint(p1, p2);
    // double p1p2Length = calculateLength(p1, p2);
    // std::cout << "Midpoint: (" << midPoint[0] << ", " << midPoint[1] << ")\n";
    // std::cout << "length:" << p1p2Length << "\n";

    // // TEST: geometry.h的interpolate
    // double start = 1;
    // double end = 2;
    // int num = 2;
    // std::vector<double> x = interpolate(start, end, num);
    // std::cout << "x: ";
    // for (auto i : x)
    // {
    //     std::cout << i << "\n";
    // }

    // // TEST: interpolatePloyline
    // MultiPoints2D pline = {{0, 0}, {1, 1}, {2, 2}, {3, 3}};
    // MultiPoints2D result = interpolatedPolyLine(pline, 5);

    // std::cout << "result size: " << result.size() << "\n";
    // for (auto p : result)
    // {
    //     std::cout << p[0] << " " << p[1] << "\n";
    // }

    // TEST: getRaidusByuChordCircum

    // 正向
    // std::array<double,2> entryPt = {4070.4639,2630.1421};
    // std::array<double,2>exitPt = {3831.7140,2570.1421};
    // std::array<double,2> mdPt = getMidPoint(entryPt,exitPt);

    // double horizontalAngle = std::atan2(entryPt[1]-exitPt[1],entryPt[0]-exitPt[0]);

    // double chordLength = calculateLength(entryPt,exitPt);
    // double chordToCircum = 30.9328;

    // double radius = getRadiusByChordCircum(chordLength,chordToCircum);

    // std::cout <<"radius:"<<radius<<"\n";

    // std::array<double,2> center = getCenter(mdPt,horizontalAngle,radius,chordToCircum);

    // 反向
    // std::array<double,2> entryPt = {4386.1388,2630.1421};
    // std::array<double,2> exitPt = {4624.8887,2570.1421};
    // std::array<double,2> mdPt = getMidPoint(entryPt,exitPt);

    // double horizontalAngle = std::atan2(entryPt[1]-exitPt[1],entryPt[0]-exitPt[0]);
    // double chordLength = calculateLength(entryPt,exitPt);
    // double chordToCircum = 30.9328;

    // double radius = getRadiusByChordCircum(chordLength,chordToCircum);
    // std::array<double,2> center = getCenter(mdPt,horizontalAngle,radius,chordToCircum);

    // TEST intersectionNum函数
    // MultiPoints2D slopeShp = {{3674.3783, 2570.1421}, {3874.3783, 2570.1421}, {3926.3398, 2600.1421}, {3976.3398, 2600.1421}, {4028.3013, 2630.1421}, {4228.3013, 2630.1421}};

    // std::array<double, 2> entryPt = {4070.4639, 2630.1421};
    // std::array<double, 2> exitPt = {3831.7140, 2570.1421};
    // std::array<double, 2> mdPt = getMidPoint(entryPt, exitPt);

    // double horizontalAngle = std::atan2(entryPt[1] - exitPt[1], entryPt[0] - exitPt[0]);

    // double chordLength = calculateLength(entryPt, exitPt);
    // double chordToCircum = 10;

    // double radius = getRadiusByChordCircum(chordLength, chordToCircum);

    // std::array<double, 2> center = getCenter(mdPt, horizontalAngle, radius, chordToCircum);

    // MultiPoints2D intersectedPline = cutFromPolyLine(slopeShp, exitPt, entryPt);

    // printPolyLine(intersectedPline);

    // int intersection = intersectionNum(slopeShp, center, radius);

    // std::cout << "intersection num:" << intersection << "\n";

    // test geometry intersectionNum
    // MultiPoints2D slopeReverse = {
    //     {4228.3013, 2630.1421},
    //     {4428.3013, 2630.1421},
    //     {4480.2629, 2600.1421},
    //     {4530.2629, 2600.1421},
    //     {4582.2244, 2570.1421},
    //     {4882.2244, 2570.1421},
    // };

    // std::array<double, 2> entryPt = {4386.1388, 2630.1421};
    // std::array<double, 2> exitPt = {4624.8887, 2570.1421};

    // std::array<double, 2> mdPt = getMidPoint(entryPt, exitPt);
    // double horizontalAngle = std::atan2(entryPt[1] - exitPt[1], entryPt[0] - exitPt[0]);
    // double chordLength = calculateLength(entryPt, exitPt);

    // double chordToCircum = 10;
    // double radius = getRadiusByChordCircum(chordLength, chordToCircum);

    // std::array<double, 2> center = getCenter(mdPt, horizontalAngle, radius, chordToCircum);

    // MultiPoints2D intersectedPline = cutFromPolyLine(slopeReverse, exitPt, entryPt);

    // int intersection = intersectionNum(intersectedPline, center, radius);

    // std::cout << "intersection num:" << intersection << "\n";

    // ----------------------------------------------------------------
    // TEST algorithm的setEntryRange
    MultiPoints2D slopeReverse = {
        {4228.3013, 2630.1421},
        {4428.3013, 2630.1421},
        {4480.2629, 2600.1421},
        {4530.2629, 2600.1421},
        {4582.2244, 2570.1421},
        {4882.2244, 2570.1421}};

    // MultiPoints2D slopeShp = {
    //     {3674.3783, 2570.1421},
    //     {3874.3783, 2570.1421},
    //     {3926.3398, 2600.1421},
    //     {3976.3398, 2600.1421},
    //     {4028.3013, 2630.1421},
    //     {4228.3013, 2630.1421}};

    // std::pair<MultiPoints2D, MultiPoints2D> entryExitRangeOrd = setEntryExitRange(slopeShp);
    // MultiPoints2D entryRangeOrd = entryExitRangeOrd.first;
    // MultiPoints2D exitRangeOrd = entryExitRangeOrd.second;

    std::pair<MultiPoints2D, MultiPoints2D> entryExitRangeRev = setEntryExitRange(slopeReverse);
    MultiPoints2D entryRangeRev = entryExitRangeRev.first;
    MultiPoints2D exitRangeRev = entryExitRangeRev.second;

    // std::cout << "entryOrd size: " << entryRangeOrd.size() << "\n";

    // TEST algorithm的setSlips

    int entryExitNum = 3;
    int slipNum = 1;
    int slicesNum = 10;
    MultiPoints2D entryPointsSet = interpolatedPolyLine(entryRangeRev, entryExitNum);
    MultiPoints2D exitPointsSet = interpolatedPolyLine(exitRangeRev, entryExitNum);
    // std::cout << "entryPointsSet size: " << entryPointsSet.size() << "\n";

    VectorMap slipSet = setSlips(entryPointsSet, exitPointsSet, slipNum, slopeReverse);

    // for (auto it = slipSet.begin(); it != slipSet.end(); it++)
    // {
    //     // first is a tuple of (i,j,k)
    //     std::cout << "slipSet key: " << "< ";
    //     std::cout << std::get<0>(it->first) << ", ";
    //     std::cout << std::get<1>(it->first) << ", ";
    //     std::cout << std::get<2>(it->first) << " >\n";
    //     std::cout << "slipSet value: ";
    //     for (int i = 0; i < it->second.size(); i++)
    //     {

    //         std::cout << it->second[i] << ", ";
    //     }
    //     std::cout << "\n";
    // }

    // TEST algorithm的setSlicesGeom
    MatrixMap sliceSet = setSlicesGeom(entryPointsSet, exitPointsSet, slipSet, slopeReverse, slicesNum);
    std::cout << "sliceSet size: " << sliceSet.size() << "\n";

        std::cout << "TEST FINISHED" << std::endl;
    return 0;
}