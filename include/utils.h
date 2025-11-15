#ifndef LEMSOLVER_UTILS_H
#define LEMSOLVER_UTILS_H

#pragma once
#include <vector>
#include <array>
#include <stdexcept>
#include <cmath>
#include <iostream>
#include <fstream>
#include <string>
#include <nlohmann/json.hpp>

// ------------------命名空间声明--------------------

using json = nlohmann::json;

// 动态数组容器，m*2的矩阵
using MultiPoints2D = std::vector<std::array<double, 2>>;
// using MultiPoints3D = std::vector<std::array<double, 3>>;

// --------------------函数声明----------------------

// 打印多边形的线条
void printPolyLine(const MultiPoints2D &poly);

// -------------------结构体声明---------------------

// 动态数组容器
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

// 保存极限平衡法计算所用的参数
struct SolverConfig
{
    int entry_exit_points_num = 20;
    int slips_num = 10;
    int slices_num = 300;
};

// 保存极限平衡法计算结果
struct SolverResult
{
    // plot elements
    MultiPoints2D entryRange;
    MultiPoints2D exitRange;
    MultiPoints2D entryPointsSet;
    MultiPoints2D exitPointsSet;
    // result elements
    std::vector<float> factorOfSafety;
    DynamicContainer arcSlipsSet;
    DynamicContainer slicesGeosSet;
};

#endif