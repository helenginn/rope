#ifndef __vagabond__Entropy__
#define __vagabond__Entropy__

#include <vector>
#include <string>
#include <mutex>

struct Entropy
{
    static std::mutex *mutex;

    ~Entropy()
    {
    }

    std::vector<std::string> start;
    std::vector<std::string> end;
    std::vector<double> total;
    std::vector<double> perRes;
};

#endif
