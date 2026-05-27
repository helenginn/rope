#include <Path.h>
#include <paths/Entropy.h>
#include <PathEntropy.h>

Entropy::Entropy(const std::vector<PathGroup> &paths, const struct FlagParameters &flagPar)
{
    _flagPar = flagPar;
    _paths = paths;
    _ticks = 0;

    for (const PathGroup &group : paths)
    {
        _starts.insert(group[0]->startInstance());
        _ends.insert(group[0]->endInstance());
        _ticks += group.size();
    }
}

void Entropy::populateHeatMap(struct EntropyForHeatMap *entropyData)
{
    std::cout << "Number of divisions: " << _flagPar.timeDivisions << std::endl; 

    entropyData->numDivisions = _flagPar.timeDivisions;

    for(int t = 0; t < _flagPar.timeDivisions; t++)
    {
        entropyData->dataMatrix.push_back(Eigen::MatrixXf::Zero(_starts.size(), _ends.size()));
    }

    for (const PathGroup &group : _paths)
    {
        std::cout << "Group: " << group[0]->startInstance()->model_id() << " to " << group[0]->endInstance()->model_id() << std::endl;

        entropyData->start.push_back(group[0]->startInstance()->model_id());
        entropyData->end.push_back(group[0]->endInstance()->model_id());

        std::pair<int, int> idxs = index(group[0]->startInstance(), group[0]->endInstance());

        std::vector<double> entropy = pathEntropyInstancePair(_flagPar.nf, group, _flagPar.timeDivisions, _flagPar.mist);

        entropyData->total.push_back(entropy);

        for (int t = 0; t < _flagPar.timeDivisions; t++)
        {
           entropyData->dataMatrix[t](idxs.first, idxs.second) = entropy[t];
            //entropyData.dataMatrix[t](idxs.second, idxs.first) = entropy[t];
        }
     
        clickTicker();
    }

    finishTicker();
}

std::vector<double> Entropy::pathEntropyInstancePair(int numPaths, std::vector<Path *> paths, int numDivisions, bool mist)
{
    PathEntropy *pE = new PathEntropy();
    std::vector<double> entropy;

    std::vector<TorsRes4NN*> torsRes = pE->getAtomsAndResidues(numPaths, paths, numDivisions);

    if (mist == false)
    {
        struct EntropyForMatrix ent4Mat = pE->calculateEntropyIndependent(numPaths, _flagPar, torsRes, numDivisions);

        entropy = ent4Mat.totalEntropy;
    }
    else
    {
        struct EntropyForMatrix ent4Mat = pE->calculateEntropyMI(numPaths, _flagPar, torsRes);

        entropy = ent4Mat.totalEntropy;
    }

    return entropy;
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
