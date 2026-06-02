#ifndef __vagabond__Entropy__
#define __vagabond__Entropy__

#include <vector>
#include <string>
#include <mutex>
#include <PathGroup.h>
#include <PathEntropy.h>
#include <Progressor.h>
#include <vagabond/utils/Eigen/Dense>

class Instance;

class Entropy : public Progressor
{
public:
    Entropy(const std::vector<PathGroup> &paths, const struct FlagParameters &flagPar);

	struct EntropyForHeatMap
	{
		int numDivisions;

		~EntropyForHeatMap()
		{
		}

		std::vector<Instance*> start;
		std::vector<Instance*> end;
		std::vector<std::vector<double>> total;
		std::vector<std::vector<double>> perRes;

		std::vector<Eigen::MatrixXf> dataMatrix;
	}; 

    void populateHeatMap(struct EntropyForHeatMap *entropyData);

    virtual const std::string progressName() const
    {
        return "Generating heatmap";
    }

    std::vector<double> pathEntropyInstancePair(int numPaths, std::vector<Path *> paths, int numDivisions, bool mist);

    struct compare_ids
    {
        bool operator()(Instance *const &a, Instance *const &b) const
        {
            return a->id() < b->id();
        }
    };

    std::pair<int, int> index(Instance *start, Instance *end);

    typedef std::set<Instance *, compare_ids> InstanceSet;

    void sortPathGroupsByInstance(std::vector<PathGroup> &paths);

    int rows()
    {
        return _starts.size();
    }

    int cols()
    {
        return _ends.size();
    }

    std::mutex &mutex()
    {
        return _mutex;
    }

private:
    std::vector<PathGroup> _paths;
    struct FlagParameters _flagPar;

    InstanceSet _starts;
    InstanceSet _ends;

    std::mutex _mutex;
};

#endif
