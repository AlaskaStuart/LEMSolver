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

int getSliceBotIndex(const std::vector<double> &sliceStratum)
{

    int botIdx = sliceStratum.size() - 1;

    while (botIdx > 2)
    {
        if (std::isnan(sliceStratum[botIdx]))
        {
            botIdx--;
        }
        else
        {
            break;
        }
    }

    return botIdx;
}

std::vector<bool> isVector1Bigger(const std::vector<double> vect1, const std::vector<double> vect2)
{
    if (vect1.size() != vect2.size())
    {
        throw std::runtime_error("vectors size must be equal.");
    }
    else
    {
        int n = vect1.size();
        std::vector<bool> result(n);
        for (int i = 0; i < n; ++i)
        {
            result[i] = (vect1[i] >= vect2[i]);
        }
        return result;
    }
}

PolyLineCAD transformFromMultiPoints2D(const MultiPoints2D &mPts)
{
    PolyLineCAD result;
    result.reserve(mPts.size());
    for (const auto &p : mPts)
    {
        std::array<double, 3> ithPt = {p[0], p[1], 0};
        result.push_back(ithPt);
    }

    return result;
}