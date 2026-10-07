//
// Created by romain on 25/09/2026.
//

#ifndef __vagabond__RotamerProxMatrix__
#define __vagabond__RotamerProxMatrix__
#include "RotamerModifier.h"
#include "vagabond/core/StructureModification.h"

class RotMatrix : public StructureModification

{
protected:
    void prepareResources();
public:
    RotMatrix(std::map<std::string,std::vector<Rotamer>> Rotamers);
    void proximityMatrix();
    std::vector<std::pair<std::string,Eigen::MatrixXf>> returnMatrices()
    {
        return _proxMatrices;
    }
    void unifiedTorsionFetcher();

    void submitJob(float weight);
    std::map<std::string,Eigen::MatrixXf> getPossibleRotamers();

private:
    std::map<std::string,std::vector<Rotamer>> _allRotamers;
    AtomGroup *_group {};
    std::vector<Parameter *> _params;
    std::vector<std::pair<std::string,Eigen::MatrixXf>> _proxMatrices {};
};
#endif
