#include <Path.h>
#include <paths/Entropy.h>
#include <PathEntropy.h>

std::mutex *Entropy::EntropyForHeatMap::mutex = new std::mutex();

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

void Entropy::populateHeatMap(struct EntropyForHeatMap entropyData)
{
    std::cout << "Number of divisions: " << _flagPar.timeDivisions << std::endl; 

    entropyData.numDivisions = _flagPar.timeDivisions;

    for(int t = 0; t < _flagPar.timeDivisions; t++)
    {
        entropyData.dataMatrix.push_back(Eigen::MatrixXf::Zero(_starts.size(), _ends.size()));
    }

    for (const PathGroup &group : _paths)
    {
        entropyData.start.push_back(group[0]->startInstance()->model_id());
        entropyData.end.push_back(group[0]->endInstance()->model_id());
        std::vector<double> entropy = pathEntropyInstancePair(_flagPar.n, group, _flagPar.timeDivisions, _flagPar.mist);

        for(int t = 0; t < _flagPar.timeDivisions; t++)
        {
            entropyData.total.push_back(entropy);
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


