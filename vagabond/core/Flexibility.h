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

class ClusterSVD;

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

    struct HBondEntity
    {
        Atom* Donor; 
        int donorIdx;
        Atom* Acceptor;
        int acceptorIdx;
        Atom* Hydrogen;     
        int hydrogenIdx;  
        float startDist;
        Atom* ParentDonor;
        Atom* ParentAcceptor;
        float AlphaAngleDist;
        float BetaAngleDist;
        float Dihedral1;     // torsion(C, D, H, A)
        float Dihedral2;     // torsion(D, H, A, AA)
        std::vector<std::pair<int,bool>> TorsionVec; // (torsionIdx. isHSide)
    }; 

    struct VdWBondEntity
    {
        Atom* Atom1;         // first atom
        int atomIdx1;        // index in blocks
        Atom* Atom2;         // second atom
        int atomIdx2;        // index in blocks
        float startDist;     // initial distance
        float contactDist;   // sum of vdW radii + tolerance
        std::vector<std::pair<int,bool>> TorsionVec;  // torsions influencing this pair
    };


    // === GUI-INTERFACED FUNCTIONS ===
    float submitJobAndRetrieve(float weight);
    // Getters for SVD components
    const Eigen::MatrixXf& getV() const { return _V; }
    const Eigen::VectorXf& getS() const { return _S; }

    void prepareResources();
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
    void loadHBondsFromManager(HBondManager* hbondManager);
    bool checkAndGetAtom(AtomGroup* atomGroup, const std::string& atomDesc, Atom*& atom);


    // === FLEXIBILITY CALCULATION ===
    void submitJob(float weight);
    void calculateFlexWeights();
    std::vector<int> getGlobalTorsionVector() const 
    {
        return std::vector<int>(_globalTorsionSet.begin(), _globalTorsionSet.end());
    }
    void calculateTorsionFlexibility();
    void buildJacobianMatrix();

    SVDResult calculateSVD() const;
    std::vector<float> assignWeightsToTorsions(const std::vector<float>& v_i,
                                const std::vector<int>& torsionVector);
    std::vector<float> extractVColumn(const Eigen::MatrixXf &V, int colIdx) const;
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
    void writeAllTorsionsToCSV(const std::string& filename);
    void setColIdx(int chosenColIdx)
    {
        _colIdx = chosenColIdx;
    }


    // === UTILITY ===
    const std::vector<HBondEntity>& getHBonds() const 
    { 
        return _hbonds; 
    }
    const std::vector<VdWBondEntity>& getVdWBonds() const 
    { 
        return _VdWBonds; 
    }
    
    int accessAtomBlock(Atom* atom);
    float calculateDistance(const glm::vec3& vector1, const glm::vec3& vector2)
    {
        return glm::length(vector1 - vector2);  
    }
    float calculateAngle(const glm::vec3& vector1, const glm::vec3& vector2);
    float calculateAngleDistance(const glm::vec3 &vector1, const glm::vec3 &vector2, const glm::vec3 &vector3);
    std::vector<std::pair<int,bool>> lastCommonAncestorIdx(int donorBlock_idx, int donorAcceptor_idx);
    bool isAncestor(int torsionBlockIdx, int atomBlockIdx);
    int rewindBlock(int &block_idx, std::vector<std::pair<int,bool>> &torsionVector, bool isHSide);
    float alphaGradientHSide(const glm::vec3 &axisA, const glm::vec3 &axisB,
                          const glm::vec3 &D, const glm::vec3 &H, const glm::vec3 &A, bool isDHBond);
    float alphaGradientASide(const glm::vec3 &axisA, const glm::vec3 &axisB,
                          const glm::vec3 &D, const glm::vec3 &H, const glm::vec3 &A);
    float betaGradientASide(const glm::vec3 &axisA, const glm::vec3 &axisB,
                                      const glm::vec3 &H, const glm::vec3 &A,
                                      const glm::vec3 &AA, bool isAABond);
    float betaGradientHSide(const glm::vec3 &axisA, const glm::vec3 &axisB,
                                      const glm::vec3 &H, const glm::vec3 &A,
                                      const glm::vec3 &AA);
    float dihedral1GradientHSide(const glm::vec3 &axisA, const glm::vec3 &axisB,
                                           const glm::vec3 &C, const glm::vec3 &D,
                                           const glm::vec3 &H, const glm::vec3 &A, bool isDHBond);
    float dihedral1GradientASide(const glm::vec3 &axisA, const glm::vec3 &axisB,
                                           const glm::vec3 &C, const glm::vec3 &D,
                                           const glm::vec3 &H, const glm::vec3 &A);
    float dihedral2GradientHSide(const glm::vec3 &axisA, const glm::vec3 &axisB,
                                           const glm::vec3 &D, const glm::vec3 &H,
                                           const glm::vec3 &A, const glm::vec3 &AA);
    float dihedral2GradientASide(const glm::vec3 &axisA, const glm::vec3 &axisB,
                                           const glm::vec3 &D, const glm::vec3 &H,
                                           const glm::vec3 &A, const glm::vec3 &AA, bool isAABond);


    // === DEBUGGING ===
    void listClashes(const std::string &filename,
                              int saved,
                              const std::vector<Atom*> &orderedAtoms,
                              int i, int j,
                              const std::vector<float> &radii);

private:
    bool _gui = false;
    std::mutex _mutex;
    bool _setup = false;
    bool _displayTargets = false;
    std::map<Atom*, int> _atom2Block;

    std::vector<HBondEntity> _hbonds;
    std::vector<VdWBondEntity> _VdWBonds;
    std::set<int> _globalTorsionSet;
    std::vector<std::vector<float>> _allTorsions;
    std::string _flexTag;

    Eigen::MatrixXf _jacobMtx;
    Eigen::MatrixXf _V;
    Eigen::VectorXf _S;

    ClusterSVD *_cluster = nullptr;
    TorsionData *_tData = nullptr;

    int _colIdx = 0;
    int _vSize = 0;

};

#endif
