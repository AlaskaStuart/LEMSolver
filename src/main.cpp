#include "solver.h"

int main()
{
    // std::cout << "Current path: " << std::filesystem::current_path() << "\n";
    // std::ifstream f("./data/sectionModel.json");
    // if (!f.is_open())
    // {
    //     std::cerr << "Error: cannot open ../../data/sectionModel.json\n";
    //     return -1;
    // }
    // nlohmann::json j;
    // f >> j;
    // Slope slope = j.get<Slope>();

    // std::cout << "Slope points: " << slope.slope_shape.size() << "\n";
    // std::cout << "Strata count: " << slope.stratum.size() << "\n";
    // std::cout << "First stratum name: " << slope.stratum[0].strata_name << "\n";

    // // 测试geometry.h的getMidPoint, calculateLength
    // std::array<double, 2> p1 = {0, 0};
    // std::array<double, 2> p2 = {1, 1};
    // std::array<double, 2> midPoint = getMidPoint(p1, p2);
    // double p1p2Length = calculateLength(p1, p2);
    // std::cout << "Midpoint: (" << midPoint[0] << ", " << midPoint[1] << ")\n";
    // std::cout << "length:" << p1p2Length << "\n";
    // // 测试geometry.h的interpolate
    // double start = 1;
    // double end = 2;
    // int num = 2;
    // std::vector<double> x = interpolate(start, end, num);
    // std::cout << "x: ";
    // for (auto i : x)
    // {
    //     std::cout << i << "\n";
    // }

    MultiPoints2D pline = {{0, 0}, {1, 1}, {2, 2}, {3, 3}};
    MultiPoints2D result = interpolatedPolyLine(pline, 5);

    std::cout << "result size: " << result.size() << "\n";
    for (auto p : result)
    {
        std::cout << p[0] << " " << p[1] << "\n";
    }

        return 0;
}