#ifndef __vagabond__Entropy__
#define __vagabond__Entropy__

#include <vector>
#include <string>
#include <mutex>

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

    void operator=(const EntropyForHeatMap &e)
    {
        std::unique_lock<std::mutex> lock(*mutex);
    }
};

#endif
