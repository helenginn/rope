#ifndef __vagabond__Flexibility__
#define __vagabond__Flexibility__

#include <algorithm>
#include <vagabond/utils/Eigen/Dense>
#include <vagabond/core/TorsionData.h>
#include "BondCalculator.h"
#include <stdlib.h>
#include <atomic>
#include <map>
#include "StructureModification.h"
#include "HBondManager.h"
#include "FlexibilityTypes.h"

class ClusterSVD;
class Model;

struct SVDResult {
    Eigen::MatrixXf U;
    Eigen::VectorXf singularValues;
    Eigen::MatrixXf V;
};

struct BoundBox {
    glm::vec3 min; // (x_min, y_min, z_min)
    glm::vec3 max; // (x_max, y_max, z_max)
};

class Flexibility : public StructureModification {
public:
    Flexibility(Instance *i);
    ~Flexibility();

    // === GUI-INTERFACED FUNCTIONS ===
    float submitJobAndRetrieve(float weight);
    // Getters for SVD components
    const Eigen::MatrixXf& getV() const { return _V; }
    const Eigen::VectorXf& getS() const { return _S; }

    void prepareResources();
    void setFixedChain(const std::vector<AtomGroup *> subsets);
    Result* getResult()
    {
        return _resources.calculator->acquireObject();
    }
    void addHBond(const HBondManager::HBondPair &hbondPair);
    void addVnWBond();

    // === FRONT-END CONTROL AND CONFIG ===
    void setGui(bool gui)
    {
        _gui = gui;
    }
    void setCluster(ClusterSVD *const &cluster, TorsionData *const &data)
    {
        _cluster = cluster;
        _tData = data;
    }
    int getVcolumns()
    {
        return _vSize;
    }

    // === HBOND HANDLING ===
    void printHBonds() const;
    void clearHBonds();
    bool validateHBondPair(const HBondManager::HBondPair &hbondPair);
    void addInternalHBond(const HBondManager::HBondPair &hbondPair);
    void addExternalHbond(const HBondManager::HBondPair &hbondPair);
    bool checkAndGetAtom(AtomGroup* atomGroup, const std::string& atomDesc, Atom*& atom);

    // === FLEXIBILITY CALCULATION ===
    void submitJob(float weight);
    void calculateFlexWeights();
    std::vector<int> getGlobalTorsionVector() const 
    {
        return std::vector<int>(_globalTorsionSet.begin(), _globalTorsionSet.end());
    }
    void calculateTorsionFlexibility();
    void buildDoFMap();
    void selectDoFMap();
    template<class BondType>
    void addConstraintsForBonds(std::vector<BondType> &entities, 
                                         const std::vector<ConstraintType> &ctypes,
                                         int &col_counter, bool isVdW);
    void buildConstraintMap();
    void writeConstraintMapToCSV(const std::string &filename);
    void newJacobian();
    void sensitivityVector();
    enum SolveMethod { Truncated, Damped };
    InfluenceResult computeInfluenceCoef(SolveMethod method = Truncated, 
                                                    double cutoff = 1e-2, 
                                                    double muScale = 1e-6);
    void clearInfluenceResults() { _influenceResults.clear(); }
    SVDResult calculateSVD() const;
    std::vector<float> assignWeightsToTorsions(const std::vector<float>& v_i);
    std::vector<float> extractVColumn(const Eigen::MatrixXf &V, int colIdx) const;

    void setTargetCoordinate(Atom *atomA, Atom *atomB);

    std::vector<std::pair<int,bool>> TorsionVec; // (torsionIdx, isHSide)

    // === OUTPUT & ANALYSIS ===
    bool checkClashes(const std::vector<Atom*> orderedAtoms, 
                                int saved,
                               const std::vector<float> &radii,
                               const std::set<std::pair<int,int>> &exclude,
                               float tolerance);
    std::vector<float> makeRadiiVec(const AtomVector &atoms);
    std::vector<glm::vec3> makePosVec(const AtomVector &atoms);
    std::set<std::pair<int,int>> makeExcList(OpSet<Atom*> &atom_set);
    std::set<std::pair<int,int>> makeExcHBonds(std::vector<Atom*> orderedAtoms, std::map<Atom*, int> indexing);
    void submitJobRandom(int colIdx);
    void setColIdx(int chosenColIdx)
    {
        _colIdx = chosenColIdx;
    }
    void writeJacobianStatsToCSV(const std::string &filename);
    void writeJacobianToCSV(const std::string &filename);
    void writeVMatrixToCSV(const std::string &filename);
    void writeSingularValuesToCSV(const std::string &filename);
    void printRigidBodyWeights(const std::vector<float> &v_i);
    void checkGammaSanity();
    std::string summariseInfuence(const InfluenceResult &r, int topN) const;
    void writeInfluenceCSVs(const InfluenceResult &r, const std::string &prefix) const;


    // === FOR SETTING UP THE CHAIN ===
    void setStudyInstance(Instance *i) { _studyInstance = i; }
    Instance *studyInstance() const { return _studyInstance ? _studyInstance : _instance;}
    std::string getChain() const;

    // === UTILITY ===
    const std::vector<HBondEntity>& getHBonds() const 
    { 
        return _hbonds; 
    }
    const std::vector<VdWBondEntity>& getVdWBonds() const 
    { 
        return _VdWBonds; 
    }
    std::vector<std::vector<float>> &getAllTorsions() { return _allTorsions; }
    int accessAtomBlock(Atom* atom);
    float calculateDistance(const glm::vec3& vector1, const glm::vec3& vector2)
    {
        return glm::length(vector1 - vector2);  
    }
    float calculateAngle(const glm::vec3& vector1, const glm::vec3& vector2);
    std::vector<std::pair<int,bool>> lastCommonAncestorIdx(int donorBlock_idx, int donorAcceptor_idx);
    int rewindBlock(int &block_idx, std::vector<std::pair<int,bool>> &torsionVector, bool isHSide);

    // === DEBUGGING ===
    void listClashes(const std::string &filename,
                              int saved,
                              const std::vector<Atom*> &orderedAtoms,
                              int i, int j,
                              const std::vector<float> &radii);
    void checkIfAtomsExist(Atom *atom1, Atom *atom2)
    {
        if (!atom1 || !atom2)
        {   std::cerr << "[ExternalHBond] Could not find atoms: "
                  << atom1 << " -> " << atom2 << std::endl;
            return;
        }
    }

    void setRunLabel(const std::string &label) { _runLabel = label; }
    void setDisplayScale(float s) { _displayScale = s; }
    float displayScale() const { return _displayScale; }

    // to delete
    void checkZeroRows();

private:

    bool _gui = false;
    std::mutex _mutex;
    Model *_model = nullptr;
    bool _setup = false;
    bool _displayTargets = false;
    Instance *_studyInstance = nullptr;
    std::map<Atom*, int> _atom2Block;
    std::string _fixedChain = "";
    std::map<int, DoF> _dofMap;
    std::map<int, DoF> _activeDoFMap;
    std::map<int, BondConstraint> _constraintMap; 
    std::map<Atom*, AtomGroup*> _atom2Group;
    Eigen::VectorXf _gamma;
    Eigen::VectorXf _lambda;
    BondEntity _targetCoordinate; 
    bool _hasTarget = false;


    std::vector<HBondEntity> _hbonds;
    std::vector<VdWBondEntity> _VdWBonds;
    std::set<int> _globalTorsionSet;
    std::vector<std::vector<float>> _allTorsions;
    // std::vector<float> _modesScales;
    std::vector<float> _modeNorm; // per mode: 1/maxAbs of that mode
    float _displayScale = 1.0f; // viewing exaggeration, set by the GUI 

    std::string _flexTag;

    Eigen::MatrixXf _jacobMtx;
    Eigen::MatrixXf _V;
    Eigen::VectorXf _S;

    ClusterSVD *_cluster = nullptr;
    TorsionData *_tData = nullptr;

    int _colIdx = 0;
    int _vSize = 0;

    // filled in by computeInfuenceCoef, read by writeRunSummary
    std::vector<InfluenceResult> _influenceResults;
    int   _lastRank = -1;
    float _lastGammaFreeRatio = -1.0f;
    float _lastCutoff = -1.0f;
    std::string _runLabel = "baseline";


    TorsionSystem extractTorsionSystem(bool excludeVdW) const;
    void rankBonds(const Eigen::VectorXd &lambdaD, InfluenceResult &r) const;



};

#endif
