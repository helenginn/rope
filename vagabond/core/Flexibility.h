#ifndef __vagabond__Flexibility__
#define __vagabond__Flexibility__

#include <algorithm>
#include <vagabond/gui/Display.h>
#include <vagabond/utils/Eigen/Dense>
#include <vagabond/core/TorsionData.h>
#include <stdlib.h>
#include <atomic>
#include "StructureModification.h"
#include "HBondManager.h"

class ClusterSVD;

class Flexibility : public Display, public StructureModification {
public:
    Flexibility(Instance *i);
    ~Flexibility();

    struct HBondEntity
    {
        Atom* Donor; 
        int donorIdx;
        Atom* Acceptor;
        int acceptorIdx;
        float startDist;
        Atom* ParentDonor;
        Atom* ParentAcceptor;
        float AlphaAngleDist;
        float BetaAngleDist;
        std::vector<int> TorsionVec;
    }; 

    // === GUI-INTERFACED FUNCTIONS ===
    float submitJobAndRetrieve(float weight);
    void generateAtomCloud();
    std::string getFlexTag()
    {
        return  _flexTag;
    }
    void setColRange(int chosen_colIdx, bool singleColumn = true)
    {
        _colIdx = chosen_colIdx;
        _useSingleColumn = singleColumn;
    }
    void addMultipleHBonds(const std::vector<HBondManager::HBondPair> &donorAcceptorPairs);
    void prepareResources();
    void addHBond(const HBondManager::HBondPair &hbondPair);

    // === FRONT-END CONTROL AND CONFIG ===
    void setChosenWeight(const float &weight)
    {
        _chosenWeight = weight;
        if (weight < 0.01 || weight > 0.99)
        {
            _chosenWeight = 0.5;
        }
    }

    void setGui(bool gui)
    {
        _gui = gui;
    }

    void setFlexTag(std::string tagFlex)
    {
        std::string _flexTag = tagFlex;
    }

    void setCluster(ClusterSVD *const &cluster, TorsionData *const &data)
    {
        _cluster = cluster;
        _tData = data;
    }

    // === HBOND HANDLING ===
    void printHBonds() const;
    void clearHBonds();
    bool validateHBondPair(const HBondManager::HBondPair &hbondPair);
    void loadHBondsFromManager(HBondManager* hbondManager);
    bool checkAndGetAtom(AtomGroup* atomGroup, const std::string& atomDesc, Atom*& atom);

    // === FLEXIBILITY CALCULATION ===
    void submitJob(float weight);
    void calculateFlexWeights();
    void calculateTorsionFlexibility(CoordManager* manager);
    void buildJacobianMatrix();
    void calculateSVD();

    // === OUTPUT & ANALYSIS ===
    void atomCloud(float weight, const AtomVector &atoms);
    void savePositionsToCSV(const std::string &filename, std::string &_flexTag, const AtomVector &atoms);
    void calculateAnisoBfactors(std::string &_flexTag, const AtomVector &atoms);
    void saveBfactorsToCSV(const std::string &filename, std::string &_flexTag, const AtomVector &atoms);

    // === UTILITY ===
    int accessAtomBlock(Atom* atom);
    float calculateDistance(const glm::vec3& vector1, const glm::vec3& vector2)
    {
        return glm::length(vector1 - vector2);  
    }
    float calculateAngle(const glm::vec3& vector1, const glm::vec3& vector2);
    float calculateAngleDistance(const glm::vec3 &vector1, const glm::vec3 &vector2, const glm::vec3 &vector3);
    std::vector<int> lastCommonAncestorIdx(int donorBlock_idx, int donorAcceptor_idx);
    int rewindBlock(int &block_idx, std::vector<int> &torsionVector);
    Eigen::Matrix3f covariance(const std::vector<glm::vec3> &samples);
    Eigen::Matrix3f calculateCovSVD(Eigen::Matrix3f covMtx);

protected:
    float _chosenWeight = 0.5;

private:
    bool _gui = false;
    std::mutex _mutex;
    bool _setup = false;
    bool _displayTargets = false;
    bool _cloudFlag = false;
    bool _useSingleColumn = false;

    std::vector<HBondEntity> _hbonds;
    std::set<int> _globalTorsionSet;
    std::vector<int> _globalTorsionVector;
    std::vector<float> _allTorsions;
    std::vector<std::vector<float>> _allTorsionsHistory;

    Eigen::MatrixXf _jacobMtx;
    Eigen::MatrixXf _U;
    Eigen::VectorXf _singularValues;
    Eigen::MatrixXf _V;
    Eigen::Matrix3f _directCov;

    ClusterSVD *_cluster = nullptr;
    TorsionData *_tData = nullptr;

    int _colIdx;
    std::string _flexTag;
};

#endif
