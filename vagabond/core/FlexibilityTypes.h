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
    AtomGroup *atoms = nullptr; // molecule this dof belongs to
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
								const glm::vec3 &axisA,
								const glm::vec3 &axisB,
								const std::vector<AtomBlock> &blocks) const;
	virtual ~BondEntity() = default; 
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
					const glm::vec3 &axisA,
					const glm::vec3 &axisB, 
					const std::vector<AtomBlock> &blocks) const override;
}; 

struct VdWBondEntity : public BondEntity
{
    float contactDist = 0.0f;   // sum of vdW radii + tolerance
};


struct HBondConstraint
{
    HBondEntity* hbond = nullptr; 
    ConstraintType type; 
    AtomGroup* donorGroup = nullptr;
    AtomGroup* acceptorGroup = nullptr; 
    int col_idx = 1;
};


#endif