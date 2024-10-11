#ifndef __vagabond__Flexibility__
#define __vagabond__Flexibility__

#include <algorithm>
#include <vagabond/gui/Display.h>
#include <vagabond/utils/Eigen/Dense>
#include <stdlib.h>
#include <atomic>
#include "StructureModification.h"


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

    void addLinkedInstances(Instance *from, Instance *to);
    float submitJobAndRetrieve(float weight);
    // virtual void setup();
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
	void prepareResources();
    void calculateTorsionFlexibility(CoordManager* specific_manager);
	void submitJob(float weight);
    void addMultipleHBonds(const std::vector<std::pair<std::string, std::string>>& donorAcceptorPairs);
    void addHBond(std::string donor, std::string acceptor);
    void printHBonds() const;
    int accessAtomBlock(Atom* atom);
    float calculateDistance(const glm::vec3& vector1, const glm::vec3& vector2) 
    {
        return glm::length(vector1 - vector2);  
    }
    float calculateAngle(const glm::vec3& vector1, const glm::vec3& vector2);
    float calculateAngleDistance(const glm::vec3 &vector1, const glm::vec3 &vector2, const glm::vec3 &vector3);
    std::vector<int> lastCommonAncestorIdx(int donorBlock_idx, int donorAcceptor_idx);
   int rewindBlock(int &block_idx, std::vector<int> &torsionVector);
   void buildJacobianMatrix();
   void calculateSVD();
   void calculateFlexWeights();
protected:
    float _chosenWeight = 0.5;;
private:
	bool _gui = false;
	bool _setup = false;
    bool _displayTargets = false;
    std::vector<HBondEntity> _hbonds;
    std::set<int> _globalTorsionSet;
    std::vector<int> _globalTorsionVector;
    std::vector<float> _allTorsions;
    Eigen::MatrixXf _jacobMtx;
    Eigen::MatrixXf _U;
    Eigen::VectorXf _singularValues;
    Eigen::MatrixXf _V;
};

#endif