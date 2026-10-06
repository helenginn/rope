#ifndef __vagabond__FlexibilityTypes__
#define __vagabond__FlexibilityTypes__

#include <vector>
#include <string>
#include "AtomGroup.h"
#include "Atom.h"
#include "derivative_functions.h"

struct AtomBlock;

enum  ConstraintType
{
    Distance, 
    AngleAlpha, 
    AngleBeta, 
    Dihedral_1, 
    Dihedral_2
};

enum DoFType
{
    Torsion, 
    TranslX,
    TranslY, 
    TranslZ,
    RotX,
    RotY, 
    RotZ
};

struct DoF // give initial values
{
    // add a reference to the atom 
    AtomGroup *atoms = nullptr; // molecule this dof belongs to
    Atom* atom = nullptr;
    DoFType type; // torsion or rb
    int idx = -1; // torsion index, unused for rb
    std::string chain;
    bool isReference = false;
};

struct BondEntity
{
    Atom* Donor = nullptr;         // first atom
    int donorIdx = -1;        // index in blocks
	Atom* Acceptor = nullptr;         // second atom
	int acceptorIdx = -1;        // index in blocks
	float startDist = 0.0f;
	std::vector<std::pair<int,bool>> TorsionVec; // (torsionIdx. isHSide)
	virtual float getDerivative(ConstraintType type,
								const DoF &dof,
                                int pivotBlockIdx,
								const std::vector<AtomBlock> &blocks) const;
	virtual ~BondEntity() = default; 

    protected: 
    struct AxisAndPositions
    {
        glm::vec3 axisA, axisB, donorPos, acceptorPos;
    };
    AxisAndPositions computeAxisAndPositions(int pivotBlockIdx,
                                             const std::vector<AtomBlock> &blocks) const;

};

struct HBondEntity : public BondEntity
{
    Atom* Hydrogen = nullptr;     
    int hydrogenIdx = -1;  
    Atom* ParentDonor = nullptr;
    Atom* ParentAcceptor = nullptr;
    float AlphaAngleDist = 0.0f;
    float BetaAngleDist = 0.0f;
    float Dihedral1 = 0.0f;     // torsion(C, D, H, A)
    float Dihedral2 = 0.0f;     // torsion(D, H, A, AA)

	virtual float getDerivative(ConstraintType type, 
					const DoF &dof,
                    int pivotBlockIdx,
					const std::vector<AtomBlock> &blocks) const override;
}; 

struct VdWBondEntity : public BondEntity
{
    float contactDist = 0.0f;   // sum of vdW radii + tolerance
};


struct BondConstraint
{
    BondEntity* hbond = nullptr; 
    ConstraintType type; 
    AtomGroup* donorGroup = nullptr;
    AtomGroup* acceptorGroup = nullptr; 
    int col_idx = 1;
    bool isVdW = false;

};

struct TorsionSystem
{
    Eigen::MatrixXd J;
    Eigen::VectorXd gamma; 
    std::vector<int> keepCols;
    bool valid = false; 
};

struct InfluenceResult
{
    struct BondScore
    {
        Atom *donor = nullptr;
        Atom *acceptor = nullptr;
        double magnitude = 0.0;
        bool sideChain = false;
    };

    std::string label; // set by the caller, e.g. "baseline"
    std::vector<double> lambda; // per constraint column
    std::vector<double> rowNorm; // per constraint column 
    std::vector<int> keepCols;
    std::vector<BondScore> ranked; // per bond, sorted descending

    std::vector<double> singularValues;
    std::vector<double> gammaProjection; // |u_i^T gamma|

    int rank = -1;
    double cutoff = -1.0;
    double mu = -1.0;
    double gammaFreeRatio = -1.0;
    double gammaNorm = 0.0;
    int nTorsionDoF = 0;
    int nConstraints = 0;
    bool truncated = true; 
    bool valid = false;  
};


#endif