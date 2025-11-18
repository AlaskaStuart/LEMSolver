#ifndef LEMSOLVER_UTILS_H
#define LEMSOLVER_UTILS_H

#pragma once
#include <vector>
#include <array>
#include <map>
#include <stdexcept>
#include <cmath>
#include <iostream>
#include <fstream>
#include <string>
#include <nlohmann/json.hpp>

// --------------------常量定义----------------------

// 精度
// 允许误差
const double EPSILON = 1e-3;

// 滑裂面的入口和出口最大长度
const double MAX_ENTRY_EXIT_LENGTH = 200;

// 初始圆弧滑面深度，数值越大，圆弧越深
const double INIT_SLIP_DEPTH_FACTOR = 1.1;

// ------------------命名空间声明--------------------

// json模块
using json = nlohmann::json;

// 动态数组容器，m*2的矩阵
using MultiPoints2D = std::vector<std::array<double, 2>>;

// using MultiPoints3D = std::vector<std::array<double, 3>>;

// 存放可变长度元素的字典
// 应用场景：复合滑面(每个滑面是多段线，个数不可知)
// 索引为 (i, j, k), 对应 (破坏点入口编号, 破坏点出口编号, 划裂面编号)
// 值是一个向量，前三个元素分别为圆弧的圆心和半径(Rx, Ry, R)
using VectorMap = std::map<std::tuple<int, int, int>, std::vector<double>>;

using MatrixMap = std::map<std::tuple<int, int, int>, CustomMatrix>;

// --------------------函数声明----------------------

// 打印多边形的线条
void printPolyLine(const MultiPoints2D &poly);

int getSliceBotIndex(const std::vector<double> &sliceStratum);

std::vector<bool> isVector1Bigger(const std::vector<double> vect1, const std::vector<double> vect2);

// -------------------结构体声明---------------------

// 动态数组容器
struct CustomMatrix
{
private:
    int m_rows, m_cols;

public:
    std::vector<double> data;
    CustomMatrix() : m_rows(0), m_cols(0), data() {}
    CustomMatrix(int r, int c)
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

    // 设置行和列
    void setRow(int r, const std::vector<double> &rowData)
    {
        for (int c = 0; c < m_cols; ++c)
            data[r * m_cols + c] = rowData[c];
    }

    void setCol(int c, const std::vector<double> &colData)
    {
        for (int r = 0; r < m_rows; ++r)
            data[r * m_cols + c] = colData[r];
    }
    std::vector<double> getRow(int r) const
    {
        std::vector<double> row(m_cols);
        for (int c = 0; c < m_cols; ++c)
            row[c] = data[r * m_cols + c];
        return row;
    }
    std::vector<double> getCol(int c) const
    {
        std::vector<double> col(m_rows);
        for (int r = 0; r < m_rows; ++r)
            col[r] = data[r * m_cols + c];
        return col;
    }
};

#endif