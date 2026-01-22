// 控制LEMSolver行为
#ifndef LEMSOLVER_SETTINGS_H
#define LEMSOLVER_SETTINGS_H

// TO-DO: 改用单例模式
// 精度
// 允许误差
constexpr double EPSILON = 1e-3;
// 最大迭代次数
constexpr int INTERATION_NUM = 100;

// 滑裂面的入口和出口最大长度
const double MAX_ENTRY_EXIT_LENGTH = 200;

// 初始圆弧滑面深度，数值越大，圆弧越深
const double INIT_SLIP_DEPTH_FACTOR = 1.1;

// 保存极限平衡法计算所用的参数

struct SolverConfig
{
    int entry_exit_points_num = 3;
    int slips_num = 1;
    int slices_num = 10;
};

#endif // LEMSOLVER_SETTINGS_H