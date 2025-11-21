#include "solver.h"

int main()
{

    // 第一步, 读取json剖面文件
    std::string jsonfilename = "./data/sectionDemoNor.json";
    // json文件生成边坡数据结构体
    Slope slpDemo = readSlopeFromJson(jsonfilename);

    // 第二步, 创建极限平衡法计算器, 读取边坡数据
    Solver solverDemo = Solver(slpDemo);

    // 第三步, 开始计算
    // 如果不专门设置参数, 默认使用的是瑞典条分法
    solverDemo.startAnalysis();

    // 第四步, 获取计算结果
    // 包括可视化交互的要素和相应的安全系数结果
    SolverResult result = solverDemo.getResult();

    // 安全系数
    auto fsMap = result.factorOfSafety;
    std::cout << fsMap.size() << std::endl;

    std::cout << "TEST FINISHED" << std::endl;
    return 0;
}