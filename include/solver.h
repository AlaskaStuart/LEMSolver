#ifndef LEMSOLVER_SLOVER_H
#define LEMSOLVER_SLOVER_H

#include "model.h"


using Arc_slips_map = std::map<std::tuple<int, int,int>,Eigen::Vector3d>;
using Complex_slips_map = std::map<std::tuple<int, int, int>,Eigen::VectorXd>;
using Slices_map = std::map<std::tuple<int, int, int>, Eigen::MatrixXd>;
// using Stratum_map = std::map<int, std::pair<Eigen::MatrixXd, Eigen::MatrixXd>>;

class Solver{
    private:

    // calculation parameters
    int entry_exit_points_num_;
    int slips_num_;
    int slices_num_;

    // visiualization variables
    Eigen::Matrix2d entry_range_;
    Eigen::Matrix2d exit_range_;

    Eigen::MatrixX2d entry_points_set_;
    Eigen::MatrixX2d exit_points_set_;

    Arc_slips_map arc_slips_map_;
    Complex_slips_map complex_slips_map_;

    Slices_map slices_map_;

    // results
    std::map<std::tuple<int, int,int>,float> analysis_results_;

    Slices_map generateSlices(const Eigen::MatrixX2d& enPts, const Eigen::MatrixX2d& exPts, const Eigen::MatrixX2d& slpShp,const Arc_slips_map& arcSlipsMap,int slicesNum);
    Slices_map generateSlices(const Eigen::MatrixX2d& enPts, const Eigen::MatrixX2d& exPts, const Eigen::MatrixX2d& slpShp,const Complex_slips_map& complexSlipsMap,int slicesNum);

    public:
    




};

#endif