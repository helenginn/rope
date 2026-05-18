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

    void populateHeatMap(struct EntropyForHeatMap entropyData);

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

    typedef std::set<Instance *, compare_ids> InstanceSet;

private:
    std::vector<PathGroup> _paths;
    struct FlagParameters _flagPar;

    InstanceSet _starts;
    InstanceSet _ends;
};

;
#endif
