#include "utils.h"

void printPolyLine(const MultiPoints2D &poly)
{
    std::cout << std::fixed << std::setprecision(2);
    for (const auto &p : poly)
    {
        std::cout << "(" << p[0] << ", " << p[1] << ") ";
    }
    std::cout << "\n";
}