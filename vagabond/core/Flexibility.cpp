#include <vagabond/core/Flexibility.h>
#include <vagabond/utils/maths.h>
#include <vagabond/c4x/Cluster.h>
#include <mutex>
#include <gemmi/elem.hpp>
#include "Instance.h"
#include "AtomGroup.h"
#include "BondSequence.h"
#include "BondCalculator.h"
#include "BondAngle.h"
#include "BondSequenceHandler.h"
#include "HBondManager.h"
#include <vagabond/core/BondCalculator.h>
#include <vagabond/utils/svd/PCA.h>
#include <vagabond/utils/Eigen/Dense>
#include "Torsion2Atomic.h"

using Eigen::MatrixXf;
using Eigen::Matrix3f;
using Eigen::VectorXf;
using Eigen::Vector3f;
using Eigen::BDCSVD;

// Initializes the Flexibility object with an instance pointer
Flexibility::Flexibility(Instance *i) 
{
    setInstance(i); 
}

Flexibility::~Flexibility() 
{
    // stopGui();
    _instance->unload();
}

// Submits a flexibility calculation job and retrieves the result
float Flexibility::submitJobAndRetrieve(float weight) 
{

    submitJob(weight);

    Result *r = _resources.calculator->acquireObject();
    r->transplantPositions(_displayTargets);

    // retrieve(); // Retrieves data, implementationin StructuralModification class
    r->destroy(); 
    return weight; 
}


// Prepares resources for flexibility calculations
void Flexibility::prepareResources() 
{
    _resources.allocateMinimum(_threads); // Allocates minimum resources

    AtomGroup *group = _instance->currentAtoms(); // Gets the current atom group
    std::vector<AtomGroup *> subsets = group->connectedGroups(); // Gets connected groups
    for (AtomGroup *subset : subsets) 
    {
        Atom *anchor = subset->chosenAnchor(); // Gets the anchor atom
        _resources.sequences->addAnchorExtension(anchor); // Adds anchor extension to sequences
    }

    _resources.sequences->setup(); // Sets up sequences
    _resources.sequences->prepareSequences(); // Prepares sequences
}


void Flexibility::calculateTorsionFlexibility() 
{
    std::cout << "Starting calculating torsion Flexibility" << std::endl;
    CoordManager* coord_manager = _resources.sequences->manager();

    auto calculateFlexibility = [this](const Coord::Get &get, const int &idx)
    {
        float jobWeight = get(0);

        if (_colIdx < 0 || _colIdx >= _allTorsions.size())
        {
            std::cerr << "[ERROR] Invalid colIdx = " << _colIdx 
                      << " with _allTorsions.size() = " 
                      << _allTorsions.size() << std::endl;
            return 0.0f;
        }
        // Start editing from here
        float rawTorsion = _allTorsions[_colIdx][idx];
        float val = _allTorsions[_colIdx][idx] * jobWeight;     
        return val;
    };

    coord_manager->setTorsionFetcher(calculateFlexibility);

    std::cout << "Finished calculating torsion Flexibility, _colIdx = " << _colIdx << std::endl;
}


// Submits a flexibility calculation job
void Flexibility::submitJob(float weight) 
{
  BaseTask *first_hook = nullptr; // Initialize first hook
  CalcTask *final_hook = nullptr; // Initialize final hook
  
  CalcTask *calc_hook = nullptr; // Initialize calc hook
  Task<BondSequence *, void *> *let_sequence_go = nullptr; // Initialize let_sequence_go
  
  BondCalculator *const &calculator = _resources.calculator; // Gets the calculator
  BondSequenceHandler *sequences = _resources.sequences; // Gets the sequences
  
  /* this final task returns the result to the pool to collect later */
  Task<Result, void *> *submit_result = calculator->actOfSubmission(0); // Submits the result
  Flag::Calc calc = Flag::Calc(Flag::DoTorsions | Flag::DoSuperpose); // Sets calculation flags
  
  sequences->calculate(calc, {weight}, &first_hook, &final_hook); // Calculates sequences
  
  BondSequence* firstSequence = sequences->getSequences()[0]; // Gets the first sequence  
  Flag::Extract gets = Flag::Extract(Flag::AtomVector); // Sets extraction flags
  
  let_sequence_go = sequences->extract(gets, submit_result, final_hook); // Extracts data
  _resources.tasks->addTask(first_hook); // Adds task to the task list
}

void Flexibility::loadHBondsFromManager(HBondManager* hbondManager) 
{
    if (hbondManager == nullptr) 
    {
        std::cerr << "Error: HBondManager is null." << std::endl;
        return;
    }
    
    auto& donorAcceptorPairs = hbondManager->getHBondPairs();
    
    // Add the HBonds into the Flexibility class
    // addMultipleHBonds(donorAcceptorPairs);
    
    std::cout << "Successfully loaded HBond pairs into Flexibility." << std::endl;
}


bool Flexibility::validateHBondPair(const HBondManager::HBondPair &hbondPair) {
    // Initialize static counters
    static int missingDonorCount = 0;
    static int missingHydrogenCount = 0;
    static int successfulValidations = 0;

    // Retrieve the current AtomGroup
    AtomGroup* atomGroup = _instance->currentAtoms();

    if (!atomGroup) {
        std::cerr << "Error: currentAtoms() returned a null pointer." << std::endl;
        return false;
    }

    // Check if donor exists
    Atom* donorAtom = atomGroup->atomByDesc(hbondPair.hydrogen);
    if (!donorAtom) {
        ++missingDonorCount;
        std::cerr << "Error: Donor atom '" << hbondPair.hydrogen 
                  << "' not found in the AtomGroup. Total missing donors: " 
                  << missingDonorCount << std::endl;
        return false;
    }

    // Check if acceptor (hydrogen) exists
    Atom* hydrogenAtom = atomGroup->atomByDesc(hbondPair.acceptor);
    if (!hydrogenAtom) {
        ++missingHydrogenCount;
        std::cerr << "Error: Acceptor (hydrogen) atom '" << hbondPair.acceptor 
                  << "' not found in the AtomGroup. Total missing hydrogens: " 
                  << missingHydrogenCount << std::endl;
        return false;
    }

    // If both atoms are found
    ++successfulValidations;
    // std::cout << "Validation successful. Total successful validations: " 
    //           << successfulValidations << std::endl;

    return true;
}

bool Flexibility::checkAndGetAtom(AtomGroup* atomGroup, const std::string& atomDesc, Atom*& atom) 
{
    atom = atomGroup->atomByDesc(atomDesc);
    if (!atom) {
        std::cerr << "Error: Atom '" << atomDesc << "' not found." << std::endl;
        return false;
    }
    return true;
}


void Flexibility::addHBond(const HBondManager::HBondPair &hbondPair) 
{
/**
 * @brief Adds a hydrogen bond (HBond) to the internal list while ensuring validity.
 *
 * This function first validates the given HBondPair to ensure that both the donor
 * and acceptor atoms exist. It then retrieves the corresponding Atom objects,
 * performs necessary error handling, and determines the relevant atom blocks.
 *
 * The function computes key hydrogen bond properties, including distances and angles,
 * and stores them in an HBondEntity. The computed torsion vector is added to the 
 * global torsion set.
 *
 * @param hbondPair The hydrogen bond pair containing donor and acceptor atom descriptors.
 */
    // Validate the HBondPair atoms
    if (!validateHBondPair(hbondPair)) {
        std::cerr << "Validation failed: One or both atoms not found. Skipping HBond addition." << std::endl;
        return;
    }

    AtomGroup* atomGroup = _instance->currentAtoms();
    Atom* acceptorAtom = atomGroup->atomByDesc(hbondPair.acceptor);
    Atom* hydrogenAtom = atomGroup->atomByDesc(hbondPair.hydrogen);

    // Error handling for acceptor
    if (!checkAndGetAtom(atomGroup, hbondPair.acceptor, acceptorAtom) || 
        !checkAndGetAtom(atomGroup, hbondPair.hydrogen, hydrogenAtom)) {
        return;
    }

    Atom* donorAtom = hydrogenAtom->connectedAtom(0); // Assuming bonded atom is donor
    if (!donorAtom) {
        std::cerr << "Error: Hydrogen atom '" << hbondPair.hydrogen << "' is not connected to any atom." << std::endl;
        return;
    }

    // Access donor and acceptor positions
    int donorBlock_idx = accessAtomBlock(donorAtom);
    int acceptorBlock_idx = accessAtomBlock(acceptorAtom);
    int hydrogenBlock_idx = accessAtomBlock(hydrogenAtom);

    // Access the donor and acceptor AtomBlock objects
    const std::vector<AtomBlock>& blocks = _resources.sequences->sequence()->blocks();
    const AtomBlock& donorBlock = blocks[donorBlock_idx];
    const AtomBlock& acceptorBlock = blocks[acceptorBlock_idx];
    const AtomBlock& hydrogenBlock = blocks[hydrogenBlock_idx];
    
    int parentDonor_idx = blocks[donorBlock_idx].parent_idx;
    int parentAcceptor_idx = blocks[acceptorBlock_idx].parent_idx;

    // Compute positions, distances, and angles
    glm::vec3 donorPos = blocks[donorBlock_idx].my_position();
    glm::vec3 acceptorPos = blocks[acceptorBlock_idx].my_position();
    glm::vec3 hydroPos = blocks[hydrogenBlock_idx].my_position();
    glm::vec3 parentDonorPos = blocks[donorBlock_idx + parentDonor_idx].my_position();
    glm::vec3 parentAcceptorPos = blocks[acceptorBlock_idx + parentAcceptor_idx].my_position();

    float distance = calculateDistance(hydroPos, acceptorPos);
    // float alphaAngleDistance = calculateAngleDistance(donorPos, acceptorPos, parentDonorPos);
    glm::vec3 u = glm::normalize(donorPos - hydroPos);   // D - H
    glm::vec3 w = glm::normalize(acceptorPos - hydroPos); // A - H
    float alphaAngleDistance = glm::dot(u, w);            // cos(D-H-A)
    // float betaAngleDistance = calculateAngleDistance(acceptorPos, donorPos, parentAcceptorPos);
    glm::vec3 ub = glm::normalize(hydroPos - acceptorPos);  // H - A
    glm::vec3 wb = glm::normalize(parentAcceptorPos - acceptorPos); // AA - A
    float betaAngleDistance = glm::dot(ub, wb); // cos(H-A-AA)

   auto torsionAngle = [](const glm::vec3& p1, const glm::vec3& p2,
                       const glm::vec3& p3, const glm::vec3& p4)
    {
        glm::vec3 b1 = p2 - p1;
        glm::vec3 b2 = p3 - p2;
        glm::vec3 b3 = p4 - p3;
        glm::vec3 n1 = glm::cross(b1, b2);
        glm::vec3 n2 = glm::cross(b2, b3);
        glm::vec3 b2_hat = glm::normalize(b2);
        float x = glm::dot(n1, n2);
        float y = glm::dot(glm::cross(n1, b2_hat), n2);
        return std::atan2(y, x);
    }; 

    // torsion(C, D, H, A) — C is parent of donor
    float dihedral1 = torsionAngle(parentDonorPos, donorPos, hydroPos, acceptorPos);

    // torsion(D, H, A, AA) — AA is parent of acceptor
    float dihedral2 = torsionAngle(donorPos, hydroPos, acceptorPos, parentAcceptorPos);




    // Create HBondEntity and store values
    HBondEntity hbe;
    hbe.Donor = donorAtom;
    hbe.donorIdx = donorBlock_idx;
    hbe.Acceptor = acceptorAtom;
    hbe.acceptorIdx = acceptorBlock_idx;
    hbe.Hydrogen = hydrogenAtom;                
    hbe.hydrogenIdx = hydrogenBlock_idx;
    hbe.startDist = distance;
    hbe.ParentDonor = blocks[donorBlock_idx + parentDonor_idx].atom;
    hbe.ParentAcceptor = blocks[acceptorBlock_idx + parentAcceptor_idx].atom;
    hbe.AlphaAngleDist = alphaAngleDistance;
    hbe.BetaAngleDist = betaAngleDistance;
    hbe.Dihedral1 = dihedral1;
    hbe.Dihedral2 = dihedral2;

    std::vector<std::pair<int,bool>> lca_idx = lastCommonAncestorIdx(donorBlock_idx, acceptorBlock_idx);
    // Insert torsion vector to _hbe 
    hbe.TorsionVec = lca_idx;
    _hbonds.push_back(hbe);
    // _globalTorsionSet.insert(hbe.TorsionVec.begin(), hbe.TorsionVec.end());
    for (auto &[torsionIdx, isHSide] : hbe.TorsionVec)
        _globalTorsionSet.insert(torsionIdx);
}

double getVdWRadius(const Atom* atom) 
{
    std::string atomName = atom->atomName();
    char elem = atomName[0]; 

    if (elem == 'C') return 1.70;
    if (elem == 'S') return 1.80;

    // not hydrophobic → return 0.0 to skip
    return 0.0;
}

void Flexibility::addVnWBond()
{
    // not sure yet if cutoff distanc is correct maybe this should change
    double cutoffD = 0.25; // from KGS: Cutoff distance for hydrophobic interactions, sum of vdW + cutoffD

    const AtomVector &atoms = _instance->currentAtoms()->atomVector();
    const std::vector<AtomBlock>& blocks = _resources.sequences->sequence()->blocks();

    for (size_t i = 0; i < atoms.size()-1; i++)
    {
        Atom *atom_i = atoms[i];
        int block_i = accessAtomBlock(atom_i);
        double r_i = getVdWRadius(atom_i);
        if (r_i <= 0.0) continue; // skip non-hydrophobic atoms
        glm::vec3 pos_i = blocks[block_i].my_position();
        

        for (size_t j = i+1; j < atoms.size(); j++)
        {
            // sequence separation
            if (j - i < 4) continue;
            Atom *atom_j = atoms[j];
            if (atom_i->hasCommonBondstraintWithAtom(atom_j)) continue;
            int block_j = accessAtomBlock(atom_j);
            double r_j = getVdWRadius(atom_j);
            if (r_j <= 0.0) continue;
            glm::vec3 pos_j = blocks[block_j].my_position();
            
            glm::vec3 diff = pos_i - pos_j;
            float dist_sq = glm::dot(diff, diff);

            double threshold = r_i + r_j + cutoffD;
            if (dist_sq >= threshold*threshold) continue;

            VdWBondEntity vdw;
            vdw.Atom1 = atom_i;
            vdw.atomIdx1 = block_i;
            vdw.Atom2 = atom_j;
            vdw.atomIdx2 = block_j;
            vdw.startDist = glm::length(diff);
            vdw.contactDist =  threshold;
            vdw.TorsionVec = lastCommonAncestorIdx(block_i, block_j);
            if (vdw.TorsionVec.empty()) continue;
            _VdWBonds.push_back(vdw);
            
        }
    }
    std::cout << "[DEBUG] VdW bonds after filtering: " 
              << _VdWBonds.size() << std::endl;

}


float Flexibility::calculateAngleDistance(const glm::vec3 &vector1, const glm::vec3 &vector2, const glm::vec3 &vector3) 
{                           
    // NOT NECESSARY!!                   
    float a = calculateDistance(vector1, vector2);
    float b = calculateDistance(vector1, vector3);

    // Calculate alpha angle
    glm::vec3 alphaVector1 = vector3 - vector1;
    glm::vec3 alphaVector2 = vector2 - vector1;
    float alphaAngle = calculateAngle(alphaVector1, alphaVector2);

    // Calculate distance between vector3 and vector2 using the Law of Cosines
    float c = sqrt(a * a + b * b - 2 * a * b * cos(glm::radians(alphaAngle)));

    return c;
}

float Flexibility::calculateAngle(const glm::vec3& vector1, const glm::vec3& vector2) 
{
    glm::vec3 normalizedVector1 = glm::normalize(vector1);
    glm::vec3 normalizedVector2 = glm::normalize(vector2);

    float dotProduct = glm::dot(normalizedVector1, normalizedVector2);
    float angleRadians = glm::acos(dotProduct);
    float angleDegrees = glm::degrees(angleRadians);

    return angleDegrees;
}

int Flexibility::accessAtomBlock(Atom* atom) 
{
    auto lookup_atom = [this, atom]()
    {
        if (_atom2Block.count(atom) == 0) 
        {
            return -1;
        }
        return _atom2Block[atom];
    };

    const std::vector<AtomBlock>& blocks = _resources.sequences->sequence()->blocks();

    if (_atom2Block.size())
    {
        return lookup_atom();
    }

    for (int i = 0; i < blocks.size(); i++) 
    {

        const AtomBlock& block = blocks[i];

        if (block.atom)
        {
            _atom2Block[block.atom] = i;
        }
    }
    return lookup_atom();
}


std::vector<std::pair<int,bool>> Flexibility::lastCommonAncestorIdx(int donorBlock_idx, int acceptorBlock_idx)
{
    std::vector<std::pair<int,bool>> torsionVector;
    const std::vector<AtomBlock>& blocks = _resources.sequences->sequence()->blocks();
    while (true)
    {
        // CASE 1: Donor is deeper -> Move Donor up
        if (blocks[donorBlock_idx].depth > blocks[acceptorBlock_idx].depth)
        {
            donorBlock_idx = rewindBlock(donorBlock_idx, torsionVector, true);
        }
        // CASE 2: Acceptor is deeper -> Move Acceptor up
        else if (blocks[donorBlock_idx].depth < blocks[acceptorBlock_idx].depth)
        {
            acceptorBlock_idx = rewindBlock(acceptorBlock_idx, torsionVector, false); 
        }
        else // equal depth 
        {
            if (donorBlock_idx == acceptorBlock_idx)
                return torsionVector;

            // rewind at the same time
            acceptorBlock_idx = rewindBlock(acceptorBlock_idx, torsionVector, false);
            donorBlock_idx = rewindBlock(donorBlock_idx, torsionVector, true);
        }

    }
}

int Flexibility::rewindBlock(int &block_idx, std::vector<std::pair<int,bool>> &torsionVector, bool isHSide)
{
    const std::vector<AtomBlock>& blocks = _resources.sequences->sequence()->blocks();
    TorsionBasis *basis = _resources.sequences->sequence()->torsionBasis();

    // check for peptide bonds   
    // If the block has torsion larger than -1, add them to the torsion vector
    if (blocks[block_idx].torsion_idx >= 0) 
    {
        Parameter *p = basis->parameter(blocks[block_idx].torsion_idx);
        if (!p->isPeptideBond())
        {
            torsionVector.push_back({blocks[block_idx].torsion_idx, isHSide});// if the hbond is between two molecules htat are 
                                                                //not conected with a common ancestor: this case           
    
        }                                                                                                // need to be handle this case
    }
    
    int blockParent_idx = blocks[block_idx].parent_idx;
    block_idx += blockParent_idx;
    return block_idx;
}

float Flexibility::alphaGradientHSide(const glm::vec3 &axisA, const glm::vec3 &axisB,
                                       const glm::vec3 &D, const glm::vec3 &H,
                                       const glm::vec3 &A, bool isDHBond)
{
    glm::vec3 axis = glm::normalize(axisB - axisA);
    glm::vec3 u = D - H;
    glm::vec3 w = A - H;
    float u_len = glm::length(u);
    float w_len = glm::length(w);
    float dot_uw = glm::dot(u, w);

    glm::vec3 dFdD = (w / (u_len * w_len)) - (dot_uw / (u_len*u_len*u_len*w_len)) * u;
    glm::vec3 dFdA = (u / (u_len * w_len)) - (dot_uw / (u_len*w_len*w_len*w_len)) * w;
    glm::vec3 dFdH = -(dFdD + dFdA);

    float deriv = glm::dot(dFdH, glm::cross(axis, (H - axisA)));
    if (!isDHBond)
        deriv += glm::dot(dFdD, glm::cross(axis, (D - axisA)));

    return deriv;
}

float Flexibility::alphaGradientASide(const glm::vec3 &axisA, const glm::vec3 &axisB,
                                       const glm::vec3 &D, const glm::vec3 &H,
                                       const glm::vec3 &A)
{
    glm::vec3 axis = glm::normalize(axisB - axisA);
    glm::vec3 u = D - H;
    glm::vec3 w = A - H;
    float u_len = glm::length(u);
    float w_len = glm::length(w);
    float dot_uw = glm::dot(u, w);

    glm::vec3 dFdA = (u / (u_len * w_len)) - (dot_uw / (u_len*w_len*w_len*w_len)) * w;
    return glm::dot(dFdA, glm::cross(axis, (A - axisA)));
}


float Flexibility::betaGradientASide(const glm::vec3 &axisA, const glm::vec3 &axisB,
                                      const glm::vec3 &H, const glm::vec3 &A,
                                      const glm::vec3 &AA, bool isAABond)
{
    // A-side: A moves, H fixed, AA is direct bond so AA does NOT move
    glm::vec3 axis = glm::normalize(axisB - axisA);
    glm::vec3 p = H - A;
    glm::vec3 r = AA - A;
    float p_len = glm::length(p);
    float r_len = glm::length(r);
    float dot_pr = glm::dot(p, r);

    glm::vec3 dFdH = (r / (p_len * r_len)) - (dot_pr / (p_len*p_len*p_len*r_len)) * p;
    glm::vec3 dFdAA = (p / (p_len * r_len)) - (dot_pr / (p_len*r_len*r_len*r_len)) * r;
    glm::vec3 dFdA =  -(dFdH + dFdAA);

    float deriv = glm::dot(dFdA, glm::cross(axis, (A - axisA)));
    if (!isAABond)
        deriv += glm::dot(dFdAA, glm::cross(axis, (AA - axisA)));
    return deriv; 
}

float Flexibility::betaGradientHSide(const glm::vec3 &axisA, const glm::vec3 &axisB,
                                      const glm::vec3 &H, const glm::vec3 &A,
                                      const glm::vec3 &AA)
{
    // Φ₃ = cos(H-A-AA), vertex at A
    // H-side: only H moves, A and AA are fixed
    glm::vec3 axis = glm::normalize(axisB - axisA);
    glm::vec3 p = H - A;
    glm::vec3 r = AA - A;
    float p_len = glm::length(p);
    float r_len = glm::length(r);
    float dot_pr = glm::dot(p, r);

    glm::vec3 dFdH  = (r / (p_len * r_len)) - (dot_pr / (p_len*p_len*p_len*r_len)) * p;

    return glm::dot(dFdH, glm::cross(axis, (H - axisA)));
}

float Flexibility::dihedral1GradientHSide(const glm::vec3 &axisA, const glm::vec3 &axisB,
                                           const glm::vec3 &C, const glm::vec3 &D,
                                           const glm::vec3 &H, const glm::vec3 &A, bool isDHBond)
{
    // τ(C,D,H,A): H-side, direct bond assumption → only H moves
    glm::vec3 axis = glm::normalize(axisB - axisA);
    glm::vec3 b1 = D - C;
    glm::vec3 b2 = H - D;
    glm::vec3 b3 = A - H;
    glm::vec3 n1 = glm::cross(b1, b2);
    glm::vec3 n2 = glm::cross(b2, b3);
    float n1_norm = glm::dot(n1, n1);
    float n2_norm = glm::dot(n2, n2);
    float b2_norm = glm::length(b2);
    glm::vec3 dTau_dC = -(b2_norm / n1_norm) * n1;
    glm::vec3 dTau_dA =  (b2_norm / n2_norm) * n2;
    glm::vec3 dTau_dD = ((glm::dot(b1,b2)/n1_norm)*n1) - ((glm::dot(b3,b2)/n2_norm)*n2);
    glm::vec3 dTau_dH = -dTau_dC - dTau_dD - dTau_dA;

    // H always moves
    float deriv = glm::dot(dTau_dH, glm::cross(axis, (H - axisA)));
    // D and C move only if torsion is above D-H bond
    if (!isDHBond)
    {
        deriv += glm::dot(dTau_dD, glm::cross(axis, (D - axisA)));
        deriv += glm::dot(dTau_dC, glm::cross(axis, (C - axisA)));
    }
    return deriv;
}

float Flexibility::dihedral1GradientASide(const glm::vec3 &axisA, const glm::vec3 &axisB,
                                           const glm::vec3 &C, const glm::vec3 &D,
                                           const glm::vec3 &H, const glm::vec3 &A)
{
    // τ(C,D,H,A): A-side, only A moves
    glm::vec3 axis = glm::normalize(axisB - axisA);
    glm::vec3 b1 = D - C;
    glm::vec3 b2 = H - D;
    glm::vec3 b3 = A - H;
    glm::vec3 n1 = glm::cross(b1, b2);
    glm::vec3 n2 = glm::cross(b2, b3);
    float n1_norm = glm::dot(n1, n1);
    float n2_norm = glm::dot(n2, n2);
    float b2_norm = glm::length(b2);
    glm::vec3 dTau_dA = (b2_norm / n2_norm) * n2;

    // only A moves
    return glm::dot(dTau_dA, glm::cross(axis, (A - axisA)));
}

float Flexibility::dihedral2GradientHSide(const glm::vec3 &axisA, const glm::vec3 &axisB,
                                           const glm::vec3 &D, const glm::vec3 &H,
                                           const glm::vec3 &A, const glm::vec3 &AA)
{
    // τ(D,H,A,AA): H-side, direct bond assumption → only H moves
    glm::vec3 axis = glm::normalize(axisB - axisA);
    glm::vec3 b1 = H - D;
    glm::vec3 b2 = A - H;
    glm::vec3 b3 = AA - A;
    glm::vec3 n1 = glm::cross(b1, b2);
    glm::vec3 n2 = glm::cross(b2, b3);
    float n1_norm = glm::dot(n1, n1);
    float n2_norm = glm::dot(n2, n2);
    float b2_norm = glm::length(b2);
    glm::vec3 dTau_dD  = -(b2_norm / n1_norm) * n1;
    glm::vec3 dTau_dAA =  (b2_norm / n2_norm) * n2;
    glm::vec3 dTau_dH  = ((glm::dot(b1,b2)/n1_norm)*n1) - ((glm::dot(b3,b2)/n2_norm)*n2);
    glm::vec3 dTau_dA  = -dTau_dD - dTau_dH - dTau_dAA;

    // only H moves
    return glm::dot(dTau_dH, glm::cross(axis, (H - axisA)));
}

float Flexibility::dihedral2GradientASide(const glm::vec3 &axisA, const glm::vec3 &axisB,
                                           const glm::vec3 &D, const glm::vec3 &H,
                                           const glm::vec3 &A, const glm::vec3 &AA, bool isAABond)
{
    // tau(D,H,A,AA): A-side, direct bond assumption → only A moves
    glm::vec3 axis = glm::normalize(axisB - axisA);
    glm::vec3 b1 = H - D;
    glm::vec3 b2 = A - H;
    glm::vec3 b3 = AA - A;
    glm::vec3 n1 = glm::cross(b1, b2);
    glm::vec3 n2 = glm::cross(b2, b3);
    float n1_norm = glm::dot(n1, n1);
    float n2_norm = glm::dot(n2, n2);
    float b2_norm = glm::length(b2);
    glm::vec3 dTau_dD  = -(b2_norm / n1_norm) * n1;
    glm::vec3 dTau_dAA =  (b2_norm / n2_norm) * n2;
    glm::vec3 dTau_dH  = ((glm::dot(b1,b2)/n1_norm)*n1) - ((glm::dot(b3,b2)/n2_norm)*n2);
    glm::vec3 dTau_dA  = -dTau_dD - dTau_dH - dTau_dAA;

    // A always moves
    float deriv = glm::dot(dTau_dA, glm::cross(axis, (A - axisA)));
    // AA moves only if torsion is above A-AA bond
    if (!isAABond)
        deriv += glm::dot(dTau_dAA, glm::cross(axis, (AA - axisA)));
    return deriv;

}



void Flexibility::buildJacobianMatrix()
{

    // COLUMNS = CONSTRAINTS
    // ROWS = DOFS
    if (_hbonds.size() == 0) 
    {
        std::cerr << "Error: No HBonds to add." << std::endl;
        return;
    }
    // [DEBUG 1] Set columns to 1 per H-bond (Distance only)
    // Ignore VdW to isolate the specific gradient function
    int numCol = 5 * _hbonds.size() + _VdWBonds.size();

    std::vector<int> torsionVector = getGlobalTorsionVector();
    int numRow = _globalTorsionSet.size();

    // set up the JacobianMatrix
    Eigen::MatrixXf jacobianMatrix(numRow, numCol);
    jacobianMatrix.setZero();

    const std::vector<AtomBlock>& blocks = _resources.sequences->sequence()->blocks();

    // --- Hydrogen bonds ---
    for (int i = 0; i < numRow; ++i) 
    {
        int torsionID = torsionVector[i];
        OpSet<int> pivotSet = _resources.sequences->sequence()->blocksForTorsionIdx(torsionID);
        std::vector<int> pivotIndices = pivotSet.toVector();
        // NEW =================================================
        for (int pivotBlockIdx : pivotIndices)
        {

            // const AtomBlock& child = blocks[pivotBlockIdx];
            // int meIdx = pivotBlockIdx + child.parent_idx;
            // const AtomBlock& me = blocks[meIdx];
            // int grandParentIdx = meIdx + me.parent_idx;
            // const AtomBlock& grandParent = blocks[grandParentIdx];
            // child.printBlock();

            //=======================================================
            for (int j = 0; j < _hbonds.size(); ++j) 
            {

                HBondEntity& hbe = _hbonds[j];
                int colBase = j * 5; // 5 constraints per Hbond

                const AtomBlock& me = blocks[pivotBlockIdx];
                glm::vec3 APos = me.my_position();
                int parentIdx = pivotBlockIdx + me.parent_idx;
                const AtomBlock& parent = blocks[parentIdx];
                glm::vec3 BPos = parent.my_position();

                glm::vec3 CPos = blocks[hbe.acceptorIdx].my_position(); 
                glm::vec3 DPos = blocks[hbe.donorIdx].my_position(); 
                glm::vec3 HPos = blocks[hbe.hydrogenIdx].my_position();


                int parentDonor_idx = blocks[hbe.donorIdx].parent_idx;
                int parentAcceptor_idx = blocks[hbe.acceptorIdx].parent_idx;

                glm::vec3 parentDonor = blocks[hbe.donorIdx + parentDonor_idx].my_position();
                glm::vec3 parentAcceptor = blocks[hbe.acceptorIdx + parentAcceptor_idx].my_position();

                // [DEBUG] Verify the correct mapping for the first element
                // if (i == 0 && j == 0)
                // {
                    // std::cout << "   > Axis-A atom:  " << me.atom->desc() << std::endl;
                    // std::cout << "   > Axis-B atom:  " << parent.atom->desc() << std::endl;
                    // std::cout << "   > Accept atom:  " << blocks[hbe.acceptorIdx].atom->desc() << std::endl;
                    // std::cout << "   > Donor atom:  " << blocks[hbe.donorIdx].atom->desc() << std::endl;
                    // std::cout << "   > Hydrogen atom:  " << blocks[hbe.hydrogenIdx].atom->desc() << std::endl;
                // }

                bool isHSide = true;
                for (auto& [tIdx, side] : hbe.TorsionVec)
                    if (tIdx == torsionID) { isHSide = side; break; }

                bool isDHBond = (blocks[hbe.hydrogenIdx].torsion_idx == torsionID);
                bool isAABond = (blocks[hbe.acceptorIdx].torsion_idx == torsionID);

                // 1) Distance H–A
                float dDist = bond_rotation_on_distance_gradient(APos, BPos, isHSide ? CPos : HPos, isHSide ? HPos : CPos);
                jacobianMatrix(i,colBase + 0) = dDist;

                // 2) Angle D-H-A
                // float dAngle1 = bond_rotation_on_angle_gradient(APos, BPos, DPos, HPos, CPos);
                float dAngle1 = isHSide 
                    ? alphaGradientHSide(APos, BPos, DPos, HPos, CPos, isDHBond) 
                    : alphaGradientASide(APos, BPos, DPos, HPos, CPos);
                jacobianMatrix(i,colBase + 1) = dAngle1;

                // 3) Angle H-A-AA
                float dAngle2 = isHSide 
                    ? betaGradientHSide(APos, BPos, HPos, CPos, parentAcceptor) 
                    : betaGradientASide(APos, BPos, HPos, CPos, parentAcceptor, isAABond);
                jacobianMatrix(i,colBase + 2) = dAngle2;

                // 4) Dihedral C-D-H-A
                float dDihedral1 = isHSide
                    ? dihedral1GradientHSide(APos, BPos, parentDonor, DPos, HPos, CPos, isDHBond)
                    : dihedral1GradientASide(APos, BPos, parentDonor, DPos, HPos, CPos);
                jacobianMatrix(i, colBase + 3) = dDihedral1;

                // 5) Dihedral D-H-A-AA
                float dDihedral2 = isHSide
                    ? dihedral2GradientHSide(APos, BPos, DPos, HPos, CPos, parentAcceptor)
                    : dihedral2GradientASide(APos, BPos, DPos, HPos, CPos, parentAcceptor, isAABond);
                jacobianMatrix(i, colBase + 4) = dDihedral2;


            }
        }
    }
    // --- VdW bonds ---
    int vdwColBase = 5 * _hbonds.size();
    for (int i = 0; i < numRow; ++i) 
    {
        int torsionID = torsionVector[i];
        OpSet<int> pivotSet = _resources.sequences->sequence()->blocksForTorsionIdx(torsionID);
        std::vector<int> pivotIndices = pivotSet.toVector();

        for (int pivotBlockIdx : pivotIndices) 
        {
            const AtomBlock& me = blocks[pivotBlockIdx];
            glm::vec3 APos = me.my_position();
            int parentIdx = pivotBlockIdx + me.parent_idx;
            glm::vec3 BPos = blocks[parentIdx].my_position();
            
            for (int j = 0; j <_VdWBonds.size(); ++j)
            {
                VdWBondEntity& vdw = _VdWBonds[j];
            
                glm::vec3 atom1Pos = blocks[vdw.atomIdx1].my_position(); 
                glm::vec3 atom2Pos = blocks[vdw.atomIdx2].my_position();

                bool isHSide;
                for (auto& [tIdx, side] : vdw.TorsionVec)
                    if (tIdx == torsionID) { isHSide = side; break; }
                float derivative = bond_rotation_on_distance_gradient(APos, BPos, isHSide 
                    ? atom2Pos : atom1Pos, isHSide ? atom1Pos : atom2Pos);
                
                jacobianMatrix(i, vdwColBase + j) = derivative;
            }
        }
    }
    _jacobMtx = jacobianMatrix;
    std::cout << "[debug] Jacobian (transpose) J_T.rows = " << _jacobMtx.rows() << std::endl;
    std::cout << "[debug] Jacobian (transpose) J_T.cols = " << _jacobMtx.cols() << std::endl;
    std::cout << "Finished building Jacobian matrix! " << std::endl;
}


SVDResult Flexibility::calculateSVD() const
{
    MatrixXf jacobMtrT = _jacobMtx.transpose();
    BDCSVD<MatrixXf> svd(jacobMtrT, Eigen::ComputeFullU | Eigen::ComputeFullV);
    std::cout << "[debug] J.rows transpose= " << jacobMtrT.rows() << std::endl;
    std::cout << "[debug] J.cols transpose= " << jacobMtrT.cols() << std::endl;
    std::cout << "[debug] singularValues size = " << svd.singularValues().size() << std::endl;
    std::cout << "[debug] _V size = " << svd.matrixV().size() << std::endl;

    int numCols = static_cast<int>(svd.matrixV().cols());
    for (int colIdx = 0; colIdx < numCols; ++colIdx)
    {
        Eigen::VectorXf violation = jacobMtrT * svd.matrixV().col(colIdx);
        std::cout << "[DEBUG] ||J*v_" << colIdx << "|| = " 
                  << violation.norm() 
                  << "  (sigma_" << colIdx << " = ";
        if (colIdx < svd.singularValues().size())
            std::cout << svd.singularValues()(colIdx);
        else
            std::cout << "0";
        std::cout << ")" << std::endl;
    }

    return {
        svd.matrixU(),
        svd.singularValues(),
        svd.matrixV()
    };
}


void Flexibility::calculateFlexWeights()
{
    std::cout << "Calculating flex weights..." << std::endl;

    SVDResult svd = calculateSVD();
    _V = svd.V;
    _S = svd.singularValues;
    _vSize = static_cast<int>(svd.V.cols());
    std::vector<int> torsionVector = getGlobalTorsionVector();
    int totalTorsionNum = _resources.sequences->torsionBasis()->parameterCount();
    if (torsionVector.size() != svd.V.rows())
    {
        std::cerr << "Error: Size mismatch between globalTorsionVector ("
                  << torsionVector.size() << ") and V rows ("
                  << svd.V.rows() << ")." << std::endl;
        return;
    }

    // ---------------------------------
    //   Store ALL torsion-weight sets
    // ---------------------------------

    _allTorsions.clear();
    _allTorsions.reserve(_vSize);
    for (int colIdx = 0; colIdx < _vSize; ++colIdx)
    {      
        std::vector<float> v_i = extractVColumn(svd.V, colIdx);
        std::vector<float> allTorsions = assignWeightsToTorsions(v_i, torsionVector);
        _allTorsions.push_back(allTorsions);

    }
    // [debug]: default to the last mode
    _colIdx = _vSize - 1;
    std::cout << "Stored " << _allTorsions.size() << " torsion-weight vectors." << std::endl;
    std::cout << "Finished calculating calculateFlexWeights, _colIdx = " << _colIdx << std::endl;
}

std::vector<float> Flexibility::extractVColumn(const Eigen::MatrixXf &V, int colIdx) const
{
    std::vector<float> column(V.rows());
    for (int rowIdx = 0; rowIdx < V.rows(); ++rowIdx)
    {
        column[rowIdx] = V(rowIdx, colIdx);
    }
    return column;
}


void Flexibility::writeAllTorsionsToCSV(const std::string& filename)
{
    std::ofstream out(filename);
    if (!out.is_open())
    {
        std::cerr << "Error: Cannot open file " << filename << std::endl;
        return;
    }

    if (_allTorsions.empty())
    {
        std::cerr << "Warning: _allTorsions is empty, nothing to write." << std::endl;
        return;
    }

    // We currently only push_back ONE vector (since you removed the loop)
    // but I keep the structure general
    const std::vector<float>& torsions = _allTorsions.back();

    // ---- Write header ----
    out << "torsion,weight\n";

    // ---- Write each torsion-weight pair ----
    for (int i = 0; i < torsions.size(); ++i)
    {
        out << i << "," << torsions[i] << "\n";
    }

    out.close();
    // std::cout << "Wrote torsion weights to " << filename << std::endl;
}


std::vector<float> Flexibility::assignWeightsToTorsions(const std::vector<float>& v_i,
                                          const std::vector<int>& torsionVector)
{
    // maps the "active torsion" vector (from the SVD) back to the "global torsion" vector (the full protein parameter list).
    int totalTorsionNum = _resources.sequences->torsionBasis()->parameterCount();
    std::vector<float> allTorsions(totalTorsionNum, 0.0f);
    for (int i = 0; i < torsionVector.size(); ++i)
    {
        int index = torsionVector[i];
        if (index < 0 || index >= totalTorsionNum)
        {
            std::cerr << "Error: Index out of bounds in globalTorsionVector: "
                      << index << std::endl;
            continue;
        }
        allTorsions[index] = v_i[i];

    }
    return allTorsions;
}



void Flexibility::clearHBonds()
{
    std::lock_guard<std::mutex> lock(_mutex); // Ensure thread safety
    _hbonds.clear();
    _globalTorsionSet.clear();  // Clear the global torsion set
    _jacobMtx = Eigen::MatrixXf(); // Reset the Jacobian matrix to an empty state
    std::cout << "Hydrogen bonds and associated data cleared in Flexibility." << std::endl;
}


void Flexibility::printHBonds() const
{
    std::cout << "HBond Entities:" << std::endl;
    for (const HBondEntity& hbe : _hbonds) 
    {
        std::cout << "  Donor: " << hbe.Donor->desc() << std::endl;
        std::cout << "  Acceptor: " << hbe.Acceptor->desc() << std::endl;
        std::cout << "  Start Distance: " << hbe.startDist << std::endl;
        // std::cout << "  Parent Donor: " << hbe.ParentDonor->desc() << std::endl;
        // std::cout << "  Parent Acceptor: " << hbe.ParentAcceptor->desc() << std::endl;
        // std::cout << "--------------------" << std::endl;
    }
}


std::vector<glm::vec3> Flexibility::makePosVec(const AtomVector &atoms)
{
    std::vector<glm::vec3> positions;
    positions.reserve(atoms.size());
    for (Atom *atom : atoms) 
    {
        // std::cout << positions.size() << " is atom " << atom->desc() << std::endl;
        glm::vec3 pos = atom->derivedPosition();
        positions.push_back(pos);
    }
    return positions;
}

std::vector<float> Flexibility::makeRadiiVec(const AtomVector &atoms)
{
    std::vector<float> radii;
    radii.reserve(atoms.size());
    for (Atom *atom : atoms) 
    {
        // Add van der Waals radius
        std::string symbol = atom->elementSymbol();
        gemmi::Element ele = gemmi::Element(symbol);
        radii.push_back(ele.vdw_r());
    }
    return radii;
}


std::set<std::pair<int,int>> Flexibility::makeExcList(OpSet<Atom*> &atom_set)
{
    std::set<std::pair<int,int>> exclude;
    std::map<Atom*, int> indexing = atom_set.indexing();
    std::vector<Atom*> orderedAtoms = atom_set.toVector();
    auto check_bondstraint = [&exclude, &indexing]<class Type>(Type *b, Atom *left_atom)
    {
        int atomCount = b->atomCount();
        int left_idx = indexing[left_atom]; 
        for (int k = 0; k < atomCount; ++k)
        {
            Atom *right_atom = b->atom(k);
            if (right_atom == left_atom) continue;
            int right_idx = indexing[right_atom];
            auto key = std::minmax(left_idx, right_idx);
            exclude.insert(key);
            // Debug print
            // std::cout << "[makeExcList] Excluding pair: " 
            //           << key.first << " - " << key.second << "\n";
        }
    };

    for (Atom *left_atom : orderedAtoms)
    {
        // go throught all the Bondstraints
        int torsionCount = left_atom->bondTorsionCount();
        // for each torsion involving left_atom 
        for (int t = 0; t < torsionCount; ++t)
        {
            BondTorsion *torsion = left_atom->bondTorsion(t);
            check_bondstraint(torsion, left_atom);
        }
        int angleCount = left_atom->bondAngleCount();
        // for each torsion involving left_atom 
        for (int t = 0; t < angleCount; ++t)
        {
            BondAngle *angle = left_atom->bondAngle(t);
            check_bondstraint(angle, left_atom);
        }

    }
    std::set<std::pair<int,int>> hbond_exclusions = makeExcHBonds(orderedAtoms, indexing);
    exclude.insert(hbond_exclusions.begin(), hbond_exclusions.end());

    return exclude;
}

std::set<std::pair<int,int>> Flexibility::makeExcHBonds(std::vector<Atom*> orderedAtoms, std::map<Atom*, int> indexing)
{
    std::set<std::pair<int,int>> excludeHAtom;
    // std::map<Atom*, int> indexing = atom_set.indexing();

    // std::vector<Atom*> orderedAtoms = atom_set.toVector();
    std::vector<glm::vec3> positions = makePosVec(orderedAtoms);
    int n = positions.size();

    
    // Typical covalent N-H ~1.0 Å, O-H ~0.96 Å. Use a small margin.
    const float OH_cutoff = 0.96f; // 1.20f;
    const float NH_cutoff = 1.00f; //1.20f; 

    for (int i = 0; i < n; ++i)
    {
        Atom *a_i = orderedAtoms[i];
        if (a_i->elementSymbol() != "H") continue;

        for (int j = 0; j < n; ++j)
        {
            if (i == j) continue; 
            Atom *a_j = orderedAtoms[j];
            std::string elem = a_j ->elementSymbol();
            if (elem != "O" && elem !="N") continue;
            // choose cutoff based on heavy atom type
            float cutoff = (elem == "O") ? OH_cutoff : NH_cutoff;
            // axis-aligned early exits
            float dx = positions[i].x - positions[j].x;
            if (std::abs(dx) > cutoff) continue;
            float dy = positions[i].y - positions[j].y;
            if (std::abs(dy) > cutoff) continue;
            float dz = positions[i].z - positions[j].z;
            if (std::abs(dz) > cutoff) continue;


            // full squared-distance check
            float dist2 = dx*dx + dy*dy + dz*dz;
            if (dist2 <= cutoff * cutoff)
            {
                auto key = std::minmax(indexing[a_i], indexing[a_j]);
                excludeHAtom.insert(key);
            }
        }
    }
    return excludeHAtom;
}



bool Flexibility::checkClashes(const std::vector<Atom*> orderedAtoms, 
                                int saved,
                               const std::vector<float> &radii,
                               const std::set<std::pair<int,int>> &exclude,
                               float tolerance)
{
    std::vector<glm::vec3> positions = makePosVec(orderedAtoms);
    using Pair = std::pair<int, int>;
    std::set<Pair> skip; 
    for (const auto &p : exclude) 
    { 
        skip.insert(std::minmax(p.first, p.second)); 
    }
    int n = positions.size();
    for (int i = 0; i < n; ++i)
    {
        for (int j = i + 1; j < n; ++j)
        {
            if (skip.count(std::minmax(i, j)) > 0) continue; 
            float limit = radii[i] + radii[j] - tolerance;
            // float collisionFactor = 0.85f; // or 0.8–0.9, tune as needed
            // float limit = collisionFactor * (radii[i] + radii[j]);

            if (limit < 0.0f) continue; //radii to small

            // caclulate coordinate differences
            float  dx = positions[i].x - positions[j].x;
            if (std::abs(dx) > limit) continue; // early exit 

            float  dy = positions[i].y - positions[j].y;
            if (std::abs(dy) > limit) continue; // early exit 

            float dz = positions[i].z - positions[j].z;
            if (std::abs(dz) > limit) continue; // early exit 

            // only now compute the squared distance
            float dist2 = dx*dx + dy*dy + dz*dz;
            if (dist2 > limit * limit) continue;

            Atom *left = orderedAtoms[i];
            Atom *right = orderedAtoms[j];
            glm::vec3 left_initial = left->otherPosition("other");
            glm::vec3 right_initial = right->otherPosition("other");
            // calculate dist between left_initial and right_intitila
            float dist_initial =  glm::length(left_initial - right_initial);
            if (dist_initial - tolerance < limit)
            {
                limit = dist_initial - tolerance;
            }
            if (dist2 > limit * limit) continue;

            std::cerr << "[checkClashes] Clash between atoms " << i << " and " << j << "\n";
            std::cerr << "[checkClashes] dist2 " << dist2 << " and limit " << limit << "\n";
            listClashes("list_clashes.csv", saved, orderedAtoms, i, j, radii);
            return false;
            
        }
    }
    return true;

}


void Flexibility::submitJobRandom(int colIdx)
{
    if (colIdx < 0 || colIdx >= _vSize) 
    {
        std::cerr << "Invalid column index for _V: " << colIdx << std::endl;
        return;
    }

    std::vector<int> torsionVector = getGlobalTorsionVector();
    std::vector<float> v_i = extractVColumn(_V, colIdx);
    std::cout << "submitJobRandom: Using column index " << colIdx 
          << " from _V (length = " << v_i.size() << ")" << std::endl;
    std::vector<float> allTorsions = assignWeightsToTorsions(v_i, torsionVector);
    std::cout << "submitJobRandom: Assigned weights to " << allTorsions.size() 
              << " torsions. First 5 weights: ";
    std::cout << std::endl;
    BaseTask *first_hook = nullptr;
    CalcTask *final_hook = nullptr;
    BondCalculator *const &calculator = _resources.calculator;
    BondSequenceHandler *sequences = _resources.sequences;

    Task<Result, void *> *submit_result = calculator->actOfSubmission(0);
    Flag::Calc calc = Flag::Calc(Flag::DoTorsions | Flag::DoSuperpose);

    sequences->calculate(calc, allTorsions, &first_hook, &final_hook);
    BondSequence* firstSequence = sequences->getSequences()[0];
    Flag::Extract gets = Flag::Extract(Flag::AtomVector);

    Task<BondSequence *, void *> *let_sequence_go = sequences->extract(gets, submit_result, final_hook);
    _resources.tasks->addTask(first_hook);
}


void Flexibility::listClashes(const std::string &filename,
                              int saved,
                              const std::vector<Atom*> &orderedAtoms,
                              int i, int j,
                              const std::vector<float> &radii)
{
    std::ofstream file(filename, std::ios::app); // append mode
    if (!file.is_open())
    {
        std::cerr << "Could not open file: " << filename << std::endl;
        return;
    }

    // Write header if this is the first clash for this sample
    file << "sample_structure_wo_vdW" << saved << "\n";

    Atom *a1 = orderedAtoms[i];
    Atom *a2 = orderedAtoms[j];

    file << a1->desc() << "," 
         << std::fixed << std::setprecision(3) << radii[i] << ","
         << a2->desc() << ","
         << std::fixed << std::setprecision(3) << radii[j] << "\n";

    file.close();
}