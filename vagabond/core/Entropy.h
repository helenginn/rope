#ifndef __vagabond__Entropy__
#define __vagabond__Entropy__

#include <vector>
#include <string>
#include <mutex>
#include <vagabond/utils/Eigen/Dense>

struct EntropyForHeatMap
{
    static std::mutex *mutex;

    int numDivisions;

    ~EntropyForHeatMap()
    {
    }

    std::vector<std::string> start;
    std::vector<std::string> end;
    std::vector<std::vector<double>> total;
    std::vector<std::vector<double>> perRes;

    std::vector<Eigen::MatrixXf> dataMatrix;
};

#endif
