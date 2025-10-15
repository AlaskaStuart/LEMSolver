// #include <iostream>
// #include <iomanip>
// #include <fstream>

#include "lem_algorithm.hpp"
#include "utils.hpp"
// #include "config.hpp"

void writeMatrixCSV(const std::string &filename, const Eigen::MatrixXd &mat)
{
    std::ofstream file(filename);
    if (!file.is_open())
    {
        throw std::runtime_error("Could not open file");
    }

    // 设置精度
    file << std::setprecision(8);

    for (int i = 0; i < mat.rows(); ++i)
    {
        for (int j = 0; j < mat.cols(); ++j)
        {
            file << mat(i, j);
            if (j != mat.cols() - 1)
            {
                file << ",";
            }
        }
        file << "\n";
    }

    file.close();
}

void writeMapCSV(const std::string filename, const VectorXd_map vector_map_data)
{
    std::ofstream file(filename);
    file << std::setprecision(8);
    if (!file.is_open())
    {
        throw std::runtime_error("Could not open file");
    }
    for (const auto &[key, values] : vector_map_data)
    {
        file << std::get<0>(key) << "," << std::get<1>(key) << "," << std::get<2>(key);
        for (const auto &val : values)
        {
            file << "," << val;
        }
        file << "\n";
    }
}

int main()
{
    std::cout << std::fixed << std::setprecision(4);

    int ENTRY_EXIT_NUM = 4;
    int SLIPS_NUM = 5;
    int SLICES_NUM = 100;

    Eigen::MatrixXd slope_geos(14, 2);

    slope_geos << 5159.399, 1115.2416,
        5259.399, 1115.2416,
        5305.595, 1145.2416,
        5335.595, 1145.2416,
        5381.7909, 1175.2416,
        5411.7909, 1175.2416,
        5457.9869, 1205.2416,
        5487.9869, 1205.2416,
        5534.1828, 1235.2416,
        5564.1828, 1235.2416,
        5610.3787, 1265.2416,
        5640.3787, 1265.2416,
        5686.5747, 1295.2416,
        5786.5747, 1295.2416;

    Eigen::MatrixXd stratum_layer_1(7, 2);
    Eigen::MatrixXd stratum_layer_2(8, 2);
    Eigen::MatrixXd stratum_layer_3(8, 2);
    Eigen::MatrixXd stratum_layer_4(5, 2);
    Eigen::MatrixXd stratum_layer_5(8, 2);
    Eigen::MatrixXd stratum_layer_6(8, 2);
    Eigen::MatrixXd stratum_layer_7(7, 2);

    stratum_layer_1 << 5011.0352, 1389.9371,
        5088.8412, 1382.8319,
        5231.1813, 1345.6974,
        5330.6116, 1329.4719,
        5470.1924, 1307.2279,
        5686.5747, 1295.2416,
        5905.8121, 1295.2416;
    stratum_layer_2 << 5011.0352, 1387.9985,
        5126.1901, 1360.0737,
        5226.833, 1324.0739,
        5365.9935, 1302.9705,
        5501.4265, 1271.9361,
        5728.8048, 1228.488,
        5864.2378, 1204.9019,
        5905.8121, 1204.9019;
    stratum_layer_3 << 5011.0352, 1303.4918,
        5187.0728, 1259.5224,
        5307.5958, 1229.7294,
        5393.3286, 1221.0398,
        5533.7316, 1180.0744,
        5689.0447, 1152.7642,
        5813.2952, 1126.6953,
        5905.8121, 1126.6953;
    stratum_layer_4 << 5500.3635, 1189.8102,
        5554.1015, 1158.1055,
        5580.0138, 1150.4198,
        5606.7358, 1145.1611,
        5653.6524, 1158.9875;
    stratum_layer_5 << 5011.0352, 1216.5611,
        5123.6834, 1180.5998,
        5202.074, 1161.7192,
        5388.2517, 1099.4833,
        5561.1309, 1054.0302,
        5705.3137, 1042.8417,
        5847.6094, 1042.8417,
        5905.8121, 1042.8417;
    stratum_layer_6 << 5011.0352, 1158.5905,
        5067.9728, 1108.6703,
        5203.9111, 1059.0009,
        5322.76, 1024.8532,
        5482.7789, 997.6903,
        5624.1547, 987.6012,
        5823.0131, 970.5274,
        5905.8121, 980.9493;
    stratum_layer_7 << 5011.0352, 932.3038,
        5111.2427, 891.5668,
        5341.5958, 873.0628,
        5606.6754, 858.0283,
        5761.7875, 858.0283,
        5899.5364, 868.4367,
        5905.8121, 868.4367;

    Stratum_map stratum_map;
    stratum_map[0] = std::make_pair(stratum_layer_1, stratum_layer_2);
    stratum_map[1] = std::make_pair(stratum_layer_2, stratum_layer_3);
    stratum_map[2] = std::make_pair(stratum_layer_3, stratum_layer_4);
    stratum_map[3] = std::make_pair(stratum_layer_3, stratum_layer_5);
    stratum_map[4] = std::make_pair(stratum_layer_5, stratum_layer_6);
    stratum_map[5] = std::make_pair(stratum_layer_6, stratum_layer_7);

    Eigen::VectorXd strata_args_0(3);
    Eigen::VectorXd strata_args_1(3);
    Eigen::VectorXd strata_args_2(3);
    Eigen::VectorXd strata_args_3(3);
    Eigen::VectorXd strata_args_4(3);
    Eigen::VectorXd strata_args_5(3);

    strata_args_0 << 17.1, 15.7, 18.4;
    strata_args_1 << 16.7, 28.5, 22.1;
    strata_args_2 << 18.5, 7.5, 30.1;
    strata_args_3 << 19.1, 15.4, 17.8;
    strata_args_4 << 17.2, 28.2, 20.3;
    strata_args_5 << 22.3, 35.7, 36.2;

    std::map<int, Eigen::VectorXd> stratum_args;
    stratum_args[0] = strata_args_0;
    stratum_args[1] = strata_args_1;
    stratum_args[2] = strata_args_2;
    stratum_args[3] = strata_args_3;
    stratum_args[4] = strata_args_4;
    stratum_args[5] = strata_args_5;

    std::pair<Eigen::MatrixXd, Eigen::MatrixXd> entry_and_exit_range =
        setSlipEntryAndExitRange(slope_geos);

    Eigen::MatrixXd entry_range = entry_and_exit_range.first;

    Eigen::MatrixXd exit_range = entry_and_exit_range.second;

    Eigen::MatrixXd entry_points_set =
        interpolateMatrix(entry_range, ENTRY_EXIT_NUM);

    Eigen::MatrixXd exit_points_set =
        interpolateMatrix(exit_range, ENTRY_EXIT_NUM);

    VectorXd_map arc_slips_set =
        setArcSlips(entry_points_set, exit_points_set, slope_geos, SLIPS_NUM);

    // writeMatrixCSV("entry_range.csv", entry_range);
    // writeMatrixCSV("exit_range.csv", exit_range);
    // writeMatrixCSV("entry_points_set.csv", entry_points_set);
    // writeMatrixCSV("exit_points_set.csv", exit_points_set);
    // writeMapCSV("arc_slips_set.csv", arc_slips_set);

    int entry_idx = 2;
    int exit_idx = 1;
    int arc_idx = 0;

    Eigen::VectorXd entry_point = entry_points_set.row(entry_idx);
    Eigen::VectorXd exit_point = exit_points_set.row(exit_idx);
    Eigen::MatrixXd interm_slope_geos =
        cutFromPolyLine(slope_geos, entry_point, exit_point);

    Eigen::VectorXd arc_slip =
        arc_slips_set[std::make_tuple(entry_idx, exit_idx, arc_idx)];
    std::cout << "arc_slip:\n"
              << arc_slip << std::endl;

    Eigen::MatrixXd slices_with_single_strata = setSlicesWithSingleStrata(
        entry_point, exit_point, interm_slope_geos, arc_slip, SLICES_NUM);

    std::cout << "slices_with_single_strata:\n"
              << slices_with_single_strata << std::endl;
    // save as csv file

    writeMatrixCSV("slices_with_single_strata.csv", slices_with_single_strata);

    Eigen::MatrixXd slices_with_stratum =
        setSlicesWithStratum(slices_with_single_strata, stratum_map);

    double result_ordinary = calculateFSOrdinary(slices_with_stratum, stratum_args);

    std::cout << "result_ordinary:\n"
              << result_ordinary << std::endl;

    return 0;
}