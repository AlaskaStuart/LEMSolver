#ifndef UTILS_H
#define UTILS_H

#pragma once
#include <vector>
#include <array>
#include <stdexcept>
#include <cmath>
#include <iostream>
#include <fstream>
#include <string>
#include <nlohmann/json.hpp>

using json = nlohmann::json;
using MultiPoints2D = std::vector<std::array<double, 2>>;
// using MultiPoints3D = std::vector<std::array<double, 3>>;

void printPolyLine(const MultiPoints2D &poly);

struct DynamicContainer
{
private:
    int m_rows, m_cols;

public:
    std::vector<double> data;
    DynamicContainer() : m_rows(0), m_cols(0), data() {}
    DynamicContainer(int r, int c)
        : m_rows(r), m_cols(c), data(r * c, 0.0)
    {
    }

    // —— 行列访问器 ——
    int rows() const { return m_rows; }
    int cols() const { return m_cols; }

    // 返回 (rows, cols)
    std::pair<int, int> shape() const
    {
        return {m_rows, m_cols};
    }

    // —— 元素访问 ——
    double &operator()(int r, int c)
    {
        return data[r * m_cols + c];
    }
    const double &operator()(int r, int c) const
    {
        return data[r * m_cols + c];
    }
};

#endif