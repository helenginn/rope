#ifndef __vagabond__Entropy__
#define __vagabond__Entropy__

#include <vector>
#include <string>
#include <mutex>
#include <vagabond/utils/Eigen/Dense>

struct EntropyForHeatMap
{
    static std::mutex *mutex;

    ~EntropyForHeatMap()
    {
    }

    std::vector<std::string> start;
    std::vector<std::string> end;
    std::vector<double> total;
    std::vector<double> perRes;

    Eigen::MatrixXf dataMatrix;
};

#endif
