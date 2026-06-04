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

    std::vector<std::string> rowNames(std::vector<Instance*> startInstances)
    {
        std::vector<std::string> rowNames;

        for(int i = 0; i < startInstances.size(); i++)
        {
            rowNames.push_back(startInstances[i]->model_id());
        }

        return rowNames;
    }

    std::vector<std::string> colNames(std::vector<Instance*> endInstances)
    {
        std::vector<std::string> colNames;

        for(int i = 0; i < endInstances.size(); i++)
        {
            colNames.push_back(endInstances[i]->model_id());
        }

        return colNames;
    }

    std::pair<std::string, std::string> names(int l, int r);

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
