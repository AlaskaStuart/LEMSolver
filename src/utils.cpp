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

// int getIndexOfLastNonNaN(const VectorMap &slice_stratum, int idx)
// {
//     if (slice_stratum.size() == 0)
//     {
//         throw std::runtime_error("Slice stratum is empty.");
//     }
//     if (idx < 0 || idx >= slice_stratum.size() - 1)
//     {
//         idx = slice_stratum.size()- 1;
//     }
//     int last_non_nan_idx = idx;
//     for (; last_non_nan_idx >= 0; last_non_nan_idx--)
//     {
//         if (!std::isnan(slice_stratum(last_non_nan_idx, 0)))
//         {
//             break;
//         }
//     }
//     return last_non_nan_idx;
// }

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