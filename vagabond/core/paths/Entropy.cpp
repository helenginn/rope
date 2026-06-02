#include <Path.h>
#include <paths/Entropy.h>
#include <PathEntropy.h>

Entropy::Entropy(const std::vector<PathGroup> &paths, const struct FlagParameters &flagPar)
{
    _flagPar = flagPar;
    _paths = paths;
    _ticks = 0;

    sortPathGroupsByInstance(_paths);

    for (const PathGroup &group : paths)
    {
        _starts.insert(group[0]->startInstance());
        _ends.insert(group[0]->endInstance());
        _ticks += group.size();
    }
}

void Entropy::populateHeatMap(struct EntropyForHeatMap *entropyData)
{
    entropyData->numDivisions = _flagPar.timeDivisions;

    for(int t = 0; t < _flagPar.timeDivisions; t++)
    {
        entropyData->dataMatrix.push_back(Eigen::MatrixXf::Constant(_starts.size(), _ends.size(), NAN));
    }

    for (const PathGroup &group : _paths)
    {
        entropyData->start.push_back(group[0]->startInstance());
        entropyData->end.push_back(group[0]->endInstance());

        std::pair<int, int> idxs = index(group[0]->startInstance(), group[0]->endInstance());
        
        {
            std::unique_lock<std::mutex> lock(mutex());
            std::vector<double> entropy = pathEntropyInstancePair(_flagPar.nf, group, _flagPar.timeDivisions, _flagPar.mist);

			std::cout << "entropy size = " << entropy.size() << std::endl;
			entropyData->total.push_back(entropy);
			std::cout << "total size = " << entropyData->total.size() << std::endl;

			for (int t = 0; t < _flagPar.timeDivisions; t++)
			{
			   entropyData->dataMatrix[t](idxs.first, idxs.second) = entropy[t];
				//entropyData.dataMatrix[t](idxs.second, idxs.first) = entropy[t];
			}
        }
     
    }
}

std::vector<double> Entropy::pathEntropyInstancePair(int numPaths, std::vector<Path *> paths, int numDivisions, bool mist)
{
    PathEntropy *pE = new PathEntropy();
    std::vector<double> entropyPair;
    
    std::cout << "Entering subroutine..." << std::endl;
    std::vector<TorsRes4NN*> torsRes = pE->getAtomsAndResidues(numPaths, paths, numDivisions);

    if (mist == false)
    {
        struct EntropyForMatrix ent4Mat = pE->calculateEntropyIndependent(numPaths, _flagPar, torsRes, numDivisions);
        std::cout << "No MIST case..." << std::endl;
        entropyPair = ent4Mat.totalEntropy;
    }
    else
    {
        struct EntropyForMatrix ent4Mat = pE->calculateEntropyMI(numPaths, _flagPar, torsRes);
        std::cout << "MIST case..." << std::endl;
        entropyPair = ent4Mat.totalEntropy;
    }

    std::cout << "Entropy total vector size: " << entropyPair.size() << std::endl;
    return entropyPair;
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
