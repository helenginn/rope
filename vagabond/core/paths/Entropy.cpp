#include <Path.h>
#include <paths/Entropy.h>
#include <PathEntropy.h>

Entropy::Entropy(const std::vector<PathGroup>& paths, const struct FlagParameters& flagPar)
{
    _flagPar = flagPar;
    _paths = paths;
    _ticks = 0;

    sortPathGroupsByInstance(_paths);

    for (const PathGroup& group : paths)
    {
        _starts.insert(group[0]->startInstance());
        _ends.insert(group[0]->endInstance());
        _ticks++;
    }
}

void Entropy::populateHeatMap(struct EntropyForHeatMap *entropyData)
{
    entropyData->numDivisions = _flagPar.timeDivisions;

    entropyData->start.clear();
    entropyData->end.clear();
    
    for (Instance *start : _starts)
    {
        entropyData->start.push_back(start);
    }

    for (Instance *end : _ends)
    {
        entropyData->end.push_back(end);
    }

    entropyData->total.resize(_paths.size());
    entropyData->perRes.resize(_paths.size());

    entropyData->dataMatrix.resize(entropyData->numDivisions);

    for(int t = 0; t < _flagPar.timeDivisions; t++)
    {
        entropyData->dataMatrix.push_back(Eigen::MatrixXf::Constant(_starts.size(), _ends.size(), NAN));
    }

    for (const PathGroup &group : _paths)
    {
        int p = 0;

        std::pair<int, int> idx = index(group[0]->startInstance(), group[0]->endInstance());
	
		std::unique_lock<std::mutex> lock(mutex());
		pathEntropyInstancePair(_flagPar.nf, group, _flagPar.timeDivisions, _flagPar.mist);

		const std::vector<EntropyResults>& results = _pathEntropy.result();

		entropyData->total[p].resize(results.size());
		entropyData->perRes[p].resize(results.size());

		for (size_t n = 0; n < results.size(); n++)
		{
			entropyData->total[p][n] = results[n].totalEntropy;
			entropyData->perRes[p][n] = results[n].entResidue;

			entropyData->dataMatrix[n](idx.first,idx.second) = results[n].totalEntropy;
		}
        
        p++;

        clickTicker();     
    }
  
    finishTicker();
}

void Entropy::pathEntropyInstancePair(int numPaths, std::vector<Path*> paths, int numDivisions, bool mist)
{
    std::vector<TorsRes4NN*> torsRes = _pathEntropy.getAtomsAndResidues(numPaths, paths, numDivisions);

    if (mist == false)
    {
        _pathEntropy.calculateEntropyIndependent(numPaths, _flagPar, torsRes, numDivisions);
    }
    else
    {
        _pathEntropy.calculateEntropyMI(numPaths, _flagPar, torsRes,numDivisions);
    }
}

void Entropy::sortPathGroupsByInstance(std::vector<PathGroup> &paths)
{
    auto compareNames = [](const PathGroup& a, const PathGroup& b)
    {
        std::string aName = a[0]->startInstance()->model_id() + " to " + a[0]->endInstance()->model_id();
        std::string bName = b[0]->startInstance()->model_id() + " to " + b[0]->endInstance()->model_id();
    
        return aName < bName;
    };

    std::sort(paths.begin(), paths.end(), compareNames);
}

std::pair<int, int> Entropy::index(Instance *start, Instance *end)
{
    int stIdx = -1;
    int endIdx = -1;

    auto fix_value = [](const Entropy::InstanceSet &insts, Instance *inst, int &idx)
    {
        idx = -1;
        int n = 0;

        for(Instance *const &check : insts)
        {
            if(inst == check)
            {
                idx = n;
                break;
            }
            n++;
        }
    };

    fix_value(_starts, start, stIdx);
    fix_value(_ends, end, endIdx);

    return {stIdx, endIdx};
}

std::pair<std::string, std::string> Entropy::names(int l, int r)
{
	std::string st_name;
	std::string end_name;

	auto fix_string = [](const Entropy::InstanceSet &insts, int idx)
	{
		int n = 0;
		for (Instance *const &check : insts)
		{
			if (n == idx)
			{
				return check->id();
			}
			n++;
		}
		return std::string("");
	};
	
	st_name = fix_string(_starts, l);
	end_name = fix_string(_ends, r);
	
	return {st_name, end_name};
}

