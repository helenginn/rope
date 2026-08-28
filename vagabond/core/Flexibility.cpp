#include <vagabond/core/Flexibility.h>
#include <vagabond/utils/maths.h>
#include <vagabond/c4x/Cluster.h>
#include <mutex>
#include <gemmi/elem.hpp>
#include "Instance.h"
#include "AtomGroup.h"
#include "FlexibilityTypes.h"
#include "BondSequence.h"
#include "matrix_functions.h"
#include "BondCalculator.h"
#include "BondAngle.h"
#include "BondSequenceHandler.h"
#include "HBondManager.h"
#include <vagabond/core/BondCalculator.h>
#include <vagabond/utils/svd/PCA.h>
#include <vagabond/utils/Eigen/Dense>
#include <vagabond/core/Model.h>
#include "Torsion2Atomic.h"




using Eigen::MatrixXf;
using Eigen::Matrix3f;
using Eigen::VectorXf;
using Eigen::Vector3f;
using Eigen::BDCSVD;

// Initializes the Flexibility object with an instance pointer
Flexibility::Flexibility(Instance *i)
: _model(i->model())
{
    setInstance(i);
}

Flexibility::~Flexibility() 
{
    // stopGui();
    delete _chainAtoms;
    _instance->unload();
}

// Submits a flexibility calculation job and retrieves the result
float Flexibility::submitJobAndRetrieve(float weight) 
{

    submitJob(weight);
    Result *r = _resources.calculator->acquireObject();
    r->transplantPositions(_displayTargets);
    r->destroy(); 
    return weight; 
}

void Flexibility::prepareResources() 
{
    _resources.allocateMinimum(_threads);
    AtomGroup* group = _model->currentAtoms();
    std::vector<AtomGroup *> subsets = group->connectedGroups();
    std::cout << "[DEBUG prepareResources] Total connected groups: " 
              << subsets.size() << std::endl;

    setReferenceMolecule(subsets);

    for (int s = 0; s < subsets.size(); s++)
    {
        Atom *anchor = subsets[s]->chosenAnchor();
        if (!anchor) continue;
        _resources.sequences->addAnchorExtension(anchor);
    }
    _resources.sequences->setup();
    _resources.sequences->prepareSequences();

    buildDoFMap();
}

void Flexibility::buildDoFMap()
{
    _dofMap.clear();
    int row_counter = 0;

    const std::vector<AtomBlock>& blocks = _resources.sequences->sequence()->blocks();
    AtomGroup *group = _model->currentAtoms();
    std::vector<AtomGroup*> subsets = group->connectedGroups();

    for (int s = 0; s < subsets.size(); s++)
    {
        Atom *anchor = subsets[s]->chosenAnchor();
        if (!anchor) continue;

        // temp solution: skip small groups (single atoms with no connectivity info)
        if (subsets[s]->size() < 3) continue;
        // this line will need to be replaced, once a more permanent solution is found


        std::string chain = anchor->chain();
        bool isReference = (chain == _referenceChain);
        bool isHetatm = anchor->hetatm(); // temp solution: will not work for all pdb files

        std::vector<std::pair<int, DoF>> chainTorsions;
        int start = -1; 
        for (int i = 0; i < blocks.size(); i++) // run throught the block until you reach the anchor and hten continue goign until you reach the nullprt
                                                    // each block is between two null pointers, what i am lookgin for is the first anchor 
        {
            if (blocks[i].atom == anchor)
            {
                start = i; 
                break;
            }
        }
        if (start < 0) continue;

        if (!isHetatm)
        {
            for (int i = start; i < blocks.size(); i++)
            {
                if (blocks[i].atom == nullptr) break;  // end of this tree
                if (blocks[i].torsion_idx < 0) continue;  // skip non-torsion blocks

                DoF dof;
                dof.atoms = subsets[s];
                dof.type = Torsion; 
                dof.atom = blocks[i].atom;
                dof.idx = blocks[i].torsion_idx;
                dof.chain = chain;
                dof.isReference = isReference;

                chainTorsions.push_back({row_counter, dof});
                _dofMap[row_counter] = dof; 
                row_counter++;
                // exits when it reachs the end pointer of the block or nullpte
            }
        }
        // rd DoFs
        if (!isReference)
        {
            for (DoFType rbType : {TranslX, TranslY, TranslZ, 
                                    RotX, RotY, RotZ})
            {
                DoF dof; 
                dof.atoms = subsets[s];
                dof.type = rbType; 
                dof.idx = -1; 
                dof.chain = chain; 
                dof.isReference = false; 
                _dofMap[row_counter] = dof; 
                row_counter++;
            }

        }   
    }
    std::cout << "[DEBUG buildDoFMap] Total DoF rows: " << row_counter << std::endl;
    std::map<std::string, int> torsionCount, rbCount;
    for (auto& [row, dof] : _dofMap)
    {
        if (dof.type == Torsion) torsionCount[dof.chain]++;
        else rbCount[dof.chain]++;
    }
    for (auto& [chain, count] : torsionCount)
        std::cout << "  Chain " << chain << " torsions: " << count << std::endl;
    for (auto& [chain, count] : rbCount)
        std::cout << "  Chain " << chain << " rigid body: " << count << std::endl;
}

void Flexibility::selectDoFMap()
{
    _activeDoFMap.clear();
    std::set<int> usedTorsions;
    for (auto &hbe : _hbonds)
        for (auto &[torsionIdx, isHSide] : hbe.TorsionVec)
            usedTorsions.insert(torsionIdx);
    for (auto &vdw : _VdWBonds)
        for (auto &[torsionIdx, isHSide] : vdw.TorsionVec)
            usedTorsions.insert(torsionIdx);

    std::set<AtomGroup*> usedGroups;
    for (auto &[col, c] : _constraintMap)
    {
        usedGroups.insert(c.donorGroup);
        usedGroups.insert(c.acceptorGroup);
    }

    int row_counter = 0;
    for (auto &[unusesoOldRow, dof] : _dofMap) // key not needed — only dof matters here
    {
        bool keep = (dof.type == Torsion)
                  ? usedTorsions.count(dof.idx) > 0  // changed from _globalTorsionSet
                  : usedGroups.count(dof.atoms) > 0; 
        if (keep)
        {
            _activeDoFMap[row_counter] = dof; // row_counter is the new index
            row_counter++; 
        }
    }

}


template<class BondType>
void Flexibility::addConstraintsForBonds(std::vector<BondType> &entities, 
                                         const std::vector<ConstraintType> &ctypes,
                                         const std::vector<AtomGroup*> &subsets,
                                         int &col_counter)
{
    for (int i = 0; i < entities.size(); i++)
    {
        BondType &entity = entities[i];     
        AtomGroup *donorGroup = nullptr; 
        AtomGroup *acceptorGroup = nullptr;


        for (AtomGroup *subset : subsets)
        {
            if (!donorGroup && subset->hasAtom(entity.Donor))
                donorGroup = subset;
            if (!acceptorGroup && subset->hasAtom(entity.Acceptor))
                acceptorGroup = subset; 
            if (donorGroup && acceptorGroup) break; 
        }

        for (ConstraintType ctype : ctypes)
        {
            BondConstraint bc;
            bc.hbond = &entity;
            bc.type = ctype; 
            bc.donorGroup = donorGroup; 
            bc.acceptorGroup = acceptorGroup;
            bc.col_idx = col_counter; 

            _constraintMap[col_counter] = bc;
            col_counter++;
        }
    }
}

void Flexibility::buildConstraintMap()
{
    _constraintMap.clear();
    int col_counter = 0;
    AtomGroup *group = _model->currentAtoms();
    std::vector<AtomGroup*> subsets = group->connectedGroups();

    addConstraintsForBonds(_hbonds, {Distance, AngleAlpha, AngleBeta, Dihedral_1, Dihedral_2}, subsets, col_counter);
    // addConstraintsForBonds(_hbonds, {Distance, AngleAlpha, AngleBeta}, subsets, col_counter);
    addConstraintsForBonds(_VdWBonds, {Distance}, subsets, col_counter);

    std::cout << "[DEBUG buildConstraintMap] Total constraint cols: " << col_counter
              << " (HBond: " << 5 * _hbonds.size() << ", VdW: " << _VdWBonds.size() << ")" << std::endl;

    writeConstraintMapToCSV("constraint_map.csv");
}



void Flexibility::setReferenceMolecule(const std::vector<AtomGroup *> subsets)
{
    // reference chain determination (largest group)
    int maxSize = 0;
    for (int s = 0; s < subsets.size(); s++)
    {
        Atom *anchor = subsets[s]->chosenAnchor();
        if (!anchor) continue;
        if (subsets[s]->size() > maxSize)
        {
            maxSize = subsets[s]->size();
            _referenceChain = anchor->chain();
        }
    }
    std::cout << "[DEBUG] Reference chain: " 
              << _referenceChain << " (" << maxSize << " atoms)" << std::endl;

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
        float scale = _modesScales[_colIdx];
        float val = _allTorsions[_colIdx][idx] * jobWeight * scale;
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


bool Flexibility::validateHBondPair(const HBondManager::HBondPair &hbondPair) {
    // Initialize static counters
    static int missingDonorCount = 0;
    static int missingHydrogenCount = 0;
    static int successfulValidations = 0;

    // Retrieve the current AtomGroup
    AtomGroup* atomGroup = _model->currentAtoms();
    // AtomGroup* atomGroup = currentChainAtoms();

    if (!atomGroup) {
        std::cerr << "Error: currentAtoms() returned a null pointer." << std::endl;
        return false;
    }

    // check hydrogen exists
    Atom* hydrogenAtom = atomGroup->atomByDesc(hbondPair.hydrogen);
    if (!hydrogenAtom)
    {
        ++missingDonorCount;
        std::cerr << "Error: Hydrogen atom '" << hbondPair.hydrogen
                  << "' not found. Total missing: " << missingDonorCount << std::endl;
        return false;
    }

    // check acceptor exists
    Atom* acceptorAtom = atomGroup->atomByDesc(hbondPair.acceptor);
    if (!acceptorAtom)
    {
        ++missingHydrogenCount;
        std::cerr << "Error: Acceptor atom '" << hbondPair.acceptor
                  << "' not found. Total missing: " << missingHydrogenCount << std::endl;
        return false;
    }

    // If both atoms are found
    ++successfulValidations;
    return true;
}

AtomGroup* Flexibility::currentChainAtoms()
{
    if (_chainAtoms) 
        { return _chainAtoms; }


    if (_targetChain.empty())
    {
        std::cout << "[DEBUG] I'm setting the chain from instance... " << std::endl;  
        AtomGroup *instGroup = _instance->currentAtoms();
        _targetChain = instGroup->atomVector()[0]->chain();
    }

    _chainAtoms = _model->currentAtoms()->new_subset([this](Atom *const &a)
    {
        return a->chain() == _targetChain;
    });

    return _chainAtoms;
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
   // Validate the HBondPair atoms
    if (!validateHBondPair(hbondPair)) return;
    addInternalHBond(hbondPair);
}

void Flexibility::addInternalHBond(const HBondManager::HBondPair &hbondPair) 
{
    // AtomGroup* atomGroup = currentChainAtoms();
    AtomGroup *atomGroup = _model->currentAtoms();
    Atom* acceptorAtom = atomGroup->atomByDesc(hbondPair.acceptor);
    Atom* hydrogenAtom = atomGroup->atomByDesc(hbondPair.hydrogen);

    if (!checkAndGetAtom(atomGroup, hbondPair.acceptor, acceptorAtom) || 
        !checkAndGetAtom(atomGroup, hbondPair.hydrogen, hydrogenAtom)) 
        { return; }

    Atom* donorAtom = hydrogenAtom->connectedAtom(0); // Assuming bonded atom is donor
    if (!donorAtom) {
        std::cerr << "Error: Hydrogen atom '" << hbondPair.hydrogen << "' is not connected to any atom." << std::endl;
        return;
    }

    int donorBlock_idx = accessAtomBlock(donorAtom);
    int acceptorBlock_idx = accessAtomBlock(acceptorAtom);
    int hydrogenBlock_idx = accessAtomBlock(hydrogenAtom);

    const std::vector<AtomBlock>& blocks = _resources.sequences->sequence()->blocks();
    // NEW: need to check that it's of these have a valid parent, 
    // otherwise the lines below will crush, cause the parent will be null
    auto hasValidParent = [&blocks](int block_idx) -> bool
    {
        if (block_idx < 0) return false;
        int parent_idx = blocks[block_idx].parent_idx;
        if (parent_idx == 0) return false;
        int parent_block = block_idx + parent_idx;
        if (parent_block < 0 || parent_block >= (int)blocks.size()) return false; 
        if (blocks[parent_block].atom == nullptr) return false;
        return true;
    };

    if (!hasValidParent(donorBlock_idx) || !hasValidParent(acceptorBlock_idx))
    {
        std::cerr << "[skip HBond] " << hbondPair.hydrogen << " -> " << hbondPair.acceptor
              << " — endpoint has no valid parent in the kinematic tree" << std::endl;
        return;      
    }
    // finished

    glm::vec3 donorPos = blocks[donorBlock_idx].my_position();
    glm::vec3 acceptorPos = blocks[acceptorBlock_idx].my_position();
    glm::vec3 hydroPos = blocks[hydrogenBlock_idx].my_position();

    int parentDonor_idx = blocks[donorBlock_idx].parent_idx;
    int parentAcceptor_idx = blocks[acceptorBlock_idx].parent_idx;

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
    // debug: flag inter-chain H-bonds
    if (hydrogenAtom->chain() != acceptorAtom->chain())
    {
        std::cout << "[DEBUG inter-chain HBond] " 
                  << hbondPair.hydrogen << " (chain " << hydrogenAtom->chain() << ")"
                  << " -> " 
                  << hbondPair.acceptor << " (chain " << acceptorAtom->chain() << ")"
                  << " TorsionVec size: " << lca_idx.size() << std::endl;
    }

    // Insert torsion vector to _hbe 
    hbe.TorsionVec = lca_idx;
    _hbonds.push_back(hbe);
    for (auto &[torsionIdx, isHSide] : hbe.TorsionVec)
    {
        _globalTorsionSet.insert(torsionIdx);
    }

}



double getVdWRadius(const Atom* atom) 
{
    std::string atomName = atom->atomName();
    char elem = atomName[0]; 

    if (elem == 'C') return 1.70;
    if (elem == 'S') return 1.80;
    return 0.0;
}

void Flexibility::addVnWBond()
{
    double cutoffD = 0.25;
    AtomGroup *atomGroup = _model->currentAtoms();
    const AtomVector &atoms = atomGroup->atomVector();
    const std::vector<AtomBlock>& blocks = _resources.sequences->sequence()->blocks();

    int skipped_commonBondstraint = 0;  // counter for hasCommonBondstraintWithAtom

    for (size_t i = 0; i < atoms.size()-1; i++)
    {
        Atom *atom_i = atoms[i];
        int block_i = accessAtomBlock(atom_i);
        double r_i = getVdWRadius(atom_i);
        if (r_i <= 0.0) continue;
        glm::vec3 pos_i = blocks[block_i].my_position();
        
        for (size_t j = i+1; j < atoms.size(); j++)
        {
            if (j - i < 4) continue;
            Atom *atom_j = atoms[j];
            if (atom_i->hasCommonBondstraintWithAtom(atom_j))
            {
                skipped_commonBondstraint++;  // count here
                continue;
            }

            if (atom_j->residueId().as_num() - atom_i->residueId().as_num() < 5) continue;

            int block_j = accessAtomBlock(atom_j);
            double r_j = getVdWRadius(atom_j);
            if (r_j <= 0.0) continue;
            glm::vec3 pos_j = blocks[block_j].my_position();
            
            glm::vec3 diff = pos_i - pos_j;
            float dist_sq = glm::dot(diff, diff);
            double threshold = r_i + r_j + cutoffD;
            if (dist_sq >= threshold*threshold) continue;

            VdWBondEntity vdw;
            vdw.Donor = atom_i;
            vdw.donorIdx = block_i;
            vdw.Acceptor = atom_j;
            vdw.acceptorIdx = block_j;
            vdw.startDist = glm::length(diff);
            vdw.contactDist = threshold;
            vdw.TorsionVec = lastCommonAncestorIdx(block_i, block_j);
            if (vdw.TorsionVec.empty()) continue;
            _VdWBonds.push_back(vdw);
        }
    }
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
        if (donorBlock_idx < 0 || acceptorBlock_idx < 0) // is this correct? 
        {
            std::cerr << "[ERROR] lastCommonAncestorIdx: no common ancestor found" << std::endl;
            return {};  // return empty vector, addHBond should skip this bond
        }
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
    // prevent infinite loop
    if (blockParent_idx == 0 && blocks[block_idx].depth == 0)
    {
        std::cerr << "[ERROR] rewindBlock: reached root at block " << block_idx;
        if (blocks[block_idx].atom)
        {
            std::cerr << " atom: " << blocks[block_idx].atom->desc();
        }
        else
        {
            std::cerr << " atom: NULL";
        }
        std::cerr << " -- no common ancestor found" << std::endl;
        return -1;
    }

    block_idx += blockParent_idx;
    return block_idx;
}

std::vector<std::pair<int, bool>> Flexibility::oneSidedTorsionVector(int chainBlock_idx)
{
    std::vector<std::pair<int, bool>> torsionVector;
    int current_idx = chainBlock_idx;
    while (current_idx >= 0)
    {
        current_idx = rewindBlock(current_idx, torsionVector, true);
    }
    return torsionVector;
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

        for (int pivotBlockIdx : pivotIndices)
        {
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

                bool isHSide = true;
                for (auto& [tIdx, side] : hbe.TorsionVec)
                    if (tIdx == torsionID) { isHSide = side; break; } // <---- HERE eror: always true

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
            
                glm::vec3 atom1Pos = blocks[vdw.donorIdx].my_position(); 
                glm::vec3 atom2Pos = blocks[vdw.acceptorIdx].my_position();

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
    std::cout << "Finished building Jacobian matrix! " << std::endl;
}


SVDResult Flexibility::calculateSVD() const
{
    MatrixXf jacobMtrT = _jacobMtx.transpose();
    BDCSVD<MatrixXf> svd(jacobMtrT, Eigen::ComputeFullU | Eigen::ComputeFullV); // remove the computation of U for now to improve speed
    // BDCSVD<MatrixXf> svd(jacobMtrT, Eigen::ComputeFullV);
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
        // svd.matrixU(),
        Eigen::MatrixXf(),  // instead of U, return empty placeholder
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

    if (_activeDoFMap.size() != svd.V.rows())
    {
        std::cerr << "Error: Size mismatch between globalTorsionVector ("
                  << _activeDoFMap.size() << ") and V rows ("
                  << svd.V.rows() << ")." << std::endl;
        return;
    }

    // ---------------------------------
    //   Store ALL torsion-weight sets
    // ---------------------------------

    _allTorsions.clear();
    _allTorsions.reserve(_vSize);
    _modesScales.clear();
    _modesScales.reserve(_vSize);


    for (int colIdx = 0; colIdx < _vSize; ++colIdx)
    {      
        std::vector<float> v_i = extractVColumn(svd.V, colIdx);
        std::vector<float> allTorsions = assignWeightsToTorsions(v_i);

        float scale = 1000.0f;

        _allTorsions.push_back(allTorsions);
        _modesScales.push_back(scale);

    }
    // [debug]: default to the last mode
    _colIdx = _vSize - 1;
    std::cout << "Stored " << _allTorsions.size() << " torsion-weight vectors." << std::endl;
    std::cout << "Finished calculating calculateFlexWeights, _colIdx = " << _colIdx << std::endl;
    writeVMatrixToCSV("V_new.csv");
    writeSingularValuesToCSV("S_new.csv");
}


void Flexibility::checkModeRBvsTorsionBudget(int colIdx)
{
    if (colIdx < 0 || colIdx >= _V.cols())
    {
        std::cerr << "[ERROR] invalid colIdx for RB/torsion budget check: " << colIdx << std::endl;
        return;
    }

    double torsionSumSq = 0.0;
    double rbSumSq = 0.0;

    for (auto &[row, dof] : _activeDoFMap)
    {
        float val = _V(row, colIdx);
        if (dof.type == Torsion)
            torsionSumSq += (double)val * val;
        else
            rbSumSq += (double)val * val;
    }

    double total = torsionSumSq + rbSumSq;
    double torsionPct = (total > 0) ? (torsionSumSq / total) * 100.0 : 0.0;
    double rbPct = (total > 0) ? (rbSumSq / total) * 100.0 : 0.0;

    std::cout << "[DEBUG budget] mode " << colIdx
              << " | torsion budget: " << torsionPct << "%"
              << " | rigid-body budget: " << rbPct << "%"
              << std::endl;
}

void Flexibility::checkModeMaxTorsion(int colIdx)
{
    if (colIdx < 0 || colIdx >= _V.cols()) return;

    float maxAbs = 0.0f;
    int maxRow = -1;
    for (auto &[row, dof] : _activeDoFMap)
    {
        if (dof.type != Torsion) continue;
        float av = std::abs(_V(row, colIdx));
        if (av > maxAbs) { maxAbs = av; maxRow = row; }
    }

    std::cout << "[DEBUG maxTorsion] mode " << colIdx
              << " maxAbs=" << maxAbs
              << " at row=" << maxRow << std::endl;
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

void Flexibility::describeTorsionLeverage(int row, int colIdx)
{
    auto it = _activeDoFMap.find(row);
    if (it == _activeDoFMap.end())
    {
        std::cerr << "[ERROR] row " << row << " not found in _activeDoFMap" << std::endl;
        return;
    }
    DoF &dof = it->second;
    if (dof.type != Torsion)
    {
        std::cout << "[DEBUG leverage] row " << row << " is not a torsion" << std::endl;
        return;
    }

    std::cout << "[DEBUG leverage] mode " << colIdx << " row " << row
              << " torsion idx=" << dof.idx
              << " atom=" << (dof.atom ? dof.atom->desc() : "null")
              << " chain=" << dof.chain
              << std::endl;

    // count how many blocks are downstream (deeper) from this torsion's pivot
    OpSet<int> pivotSet = _resources.sequences->sequence()->blocksForTorsionIdx(dof.idx);
    const std::vector<AtomBlock>& blocks = _resources.sequences->sequence()->blocks();
    for (int pivotBlockIdx : pivotSet.toVector())
    {
        int pivotDepth = blocks[pivotBlockIdx].depth;
        int downstreamCount = 0;
        for (int i = 0; i < (int)blocks.size(); i++)
        {
            if (blocks[i].atom && blocks[i].depth > pivotDepth)
                downstreamCount++;
        }
        std::cout << "  pivot block " << pivotBlockIdx 
                  << " depth=" << pivotDepth
                  << " downstream atoms (rough count)=" << downstreamCount
                  << std::endl;
    }
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


std::vector<float> Flexibility::assignWeightsToTorsions(const std::vector<float>& v_i)
{
    // maps the "active torsion" vector (from the SVD) back to the "global torsion" vector (the full protein parameter list).
    int totalTorsionNum = _resources.sequences->torsionBasis()->parameterCount();
    std::vector<float> allTorsions(totalTorsionNum, 0.0f);
    for (auto &[row, dof] : _activeDoFMap)
    {
        // if (dof.type != Torsion) continue; // RB skipped for now
        if (dof.idx < 0 || dof.idx >= totalTorsionNum)
        {
            std::cerr << "Error: Index out of bounds in globalTorsionVector: "
                      << dof.idx << std::endl;
            continue;
        }
        allTorsions[dof.idx] = v_i[row];

    }
    return allTorsions;
}



void Flexibility::clearHBonds()
{
    std::lock_guard<std::mutex> lock(_mutex);
    _hbonds.clear();
    _constraintMap.clear();
    _VdWBonds.clear();       // NEW
    _globalTorsionSet.clear();
    _atom2Block.clear();     // NEW
    _jacobMtx = Eigen::MatrixXf();
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
    }
}


std::vector<glm::vec3> Flexibility::makePosVec(const AtomVector &atoms)
{
    std::vector<glm::vec3> positions;
    positions.reserve(atoms.size());
    for (Atom *atom : atoms) 
    {
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

    // std::vector<int> torsionVector = getGlobalTorsionVector();
    std::vector<float> v_i = extractVColumn(_V, colIdx);
    std::cout << "submitJobRandom: Using column index " << colIdx 
          << " from _V (length = " << v_i.size() << ")" << std::endl;
    std::vector<float> allTorsions = assignWeightsToTorsions(v_i); // was (v_i, torsionVector)
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

static glm::vec3 axisDirectionForDoF(DoFType type)
{
    switch (type)
    {
        case TranslX: case RotX: return glm::vec3(1,0,0);
        case TranslY: case RotY: return glm::vec3(0,1,0);
        case TranslZ: case RotZ: return glm::vec3(0,0,1);
        default: return glm::vec3(0,0,0);
    }   
    return {};
}


float BondEntity::getDerivative(ConstraintType type,
                                const DoF &dof,
                                int pivotBlockIdx,
                                const std::vector<AtomBlock> &blocks) const
{
    if (dof.type == Torsion)
    {
        bool foundTorsion = false;
        bool isHSide = true;
        for (auto& [tIdx, side] : TorsionVec)
            if (tIdx == dof.idx) { foundTorsion = true; isHSide = side; break; }
        if (!foundTorsion) return 0.0f;

        AxisAndPositions p = computeAxisAndPositions(pivotBlockIdx, blocks);
        switch (type)
        {
            case Distance:
                return isHSide ? bond_rotation_on_distance_gradient(p.axisA, p.axisB,
                                                          p.acceptorPos, p.donorPos) :
                                 bond_rotation_on_distance_gradient(p.axisA, p.axisB,
                                                          p.donorPos, p.acceptorPos);

            default:
                return 0.0f;
        }
    }
    // for RB DoF
    if (type != Distance) return 0.0f;  // cause VdW only have distance cosntraints

    bool donorInGroup = dof.atoms->hasAtom(Donor);
    bool acceptorInGroup = dof.atoms->hasAtom(Acceptor);
    if (donorInGroup == acceptorInGroup) return 0.0f;
    bool isHSide = donorInGroup;

    glm::vec3 donorPos = blocks[donorIdx].my_position();
    glm::vec3 acceptorPos = blocks[acceptorIdx].my_position();
    glm::vec3 direction = axisDirectionForDoF(dof.type);

    bool isTranslation = (dof.type == TranslX || dof.type == TranslY || dof.type == TranslZ);
    if (isTranslation)
    {
        return distanceGradientTranslation(direction, donorPos, acceptorPos, isHSide);
    }
    else // rotation
    {
        glm::vec3 p_c = dof.atoms->derivedCentre();
        glm::vec3 axisA = p_c;
        glm::vec3 axisB = p_c + direction;
        return isHSide ?
            bond_rotation_on_distance_gradient(axisA, axisB, acceptorPos, donorPos) : 
            bond_rotation_on_distance_gradient(axisA, axisB, donorPos, acceptorPos);
    } 
    return 0.0f;
}

float HBondEntity::getDerivative(ConstraintType type,
                                 const DoF &dof,
                                 int pivotBlockIdx,
                                 const std::vector<AtomBlock> &blocks) const
{

    if (dof.type == Torsion)
    {
        bool foundTorsion = false; 
        bool isHSide = true; 
        for (auto &[tIdx, side] : TorsionVec)
        {
            if (tIdx == dof.idx) { foundTorsion = true; isHSide = side; break; }
        }
        if (!foundTorsion) return 0.0f;  // <-- HERE: in this version it returns 0 if the torsion is not found
                                         // if i remove this i get the same resutls as in the old Jacobian 


        AxisAndPositions p = computeAxisAndPositions(pivotBlockIdx, blocks);

        glm::vec3 hydrogenPos = blocks[hydrogenIdx].my_position();
        glm::vec3 parentDonorPos = blocks[donorIdx + blocks[donorIdx].parent_idx].my_position();
        glm::vec3 parentAcceptorPos = blocks[acceptorIdx + blocks[acceptorIdx].parent_idx].my_position();

        bool isDHBond = (blocks[hydrogenIdx].torsion_idx == dof.idx);
        bool isAABond = (blocks[acceptorIdx].torsion_idx == dof.idx);

        switch (type)
        {
            case Distance:  // Distance H–A
                return isHSide ?
                      bond_rotation_on_distance_gradient(p.axisA, p.axisB, p.acceptorPos, hydrogenPos)
                    : bond_rotation_on_distance_gradient(p.axisA, p.axisB, hydrogenPos, p.acceptorPos);
            case AngleAlpha: // Angle D-H-A
                return isHSide ?
                      alphaGradientHSide(p.axisA, p.axisB, p.donorPos, hydrogenPos, p.acceptorPos, isDHBond)
                    : alphaGradientASide(p.axisA, p.axisB, p.donorPos, hydrogenPos, p.acceptorPos);
            case AngleBeta: // Angle H-A-AA
                return isHSide ?
                      betaGradientHSide(p.axisA, p.axisB, hydrogenPos, p.acceptorPos, parentAcceptorPos)
                    : betaGradientASide(p.axisA, p.axisB, hydrogenPos, p.acceptorPos, parentAcceptorPos, isAABond);

            case Dihedral_1: // Dihedral C-D-H-A
                return isHSide ?
                      dihedral1GradientHSide(p.axisA, p.axisB, parentDonorPos, p.donorPos, hydrogenPos, p.acceptorPos, isDHBond)
                    : dihedral1GradientASide(p.axisA, p.axisB, parentDonorPos, p.donorPos, hydrogenPos, p.acceptorPos);
            case Dihedral_2: // Dihedral D-H-A-AA
                return isHSide ?
                  dihedral2GradientHSide(p.axisA, p.axisB, p.donorPos, hydrogenPos, p.acceptorPos, parentAcceptorPos)
                : dihedral2GradientASide(p.axisA, p.axisB, p.donorPos, hydrogenPos, p.acceptorPos, parentAcceptorPos, isAABond);
            default:
                return 0.0f;
        }
    }
    // rigid body dof
    bool hydrogenInGroup = dof.atoms->hasAtom(Hydrogen);
    bool acceptorInGroup = dof.atoms->hasAtom(Acceptor);
    if (hydrogenInGroup == acceptorInGroup) return 0.0f;
    bool isHSide = hydrogenInGroup;

    glm::vec3 hydrogenPos = blocks[hydrogenIdx].my_position();
    glm::vec3 donorPos = blocks[donorIdx].my_position();
    glm::vec3 acceptorPos = blocks[acceptorIdx].my_position();
    glm::vec3 parentDonorPos = blocks[donorIdx + blocks[donorIdx].parent_idx].my_position();
    glm::vec3 parentAcceptorPos = blocks[acceptorIdx + blocks[acceptorIdx].parent_idx].my_position();

    glm::vec3 direction = axisDirectionForDoF(dof.type);
    bool isTranslation = (dof.type == TranslX || dof.type == TranslY || dof.type == TranslZ);

    if (isTranslation)
    {
        switch (type)
        {
            case Distance:  // Distance H–A
                return distanceGradientTranslation(direction, hydrogenPos, acceptorPos, isHSide);
            case AngleAlpha: // Angle D-H-A
                return alphaGradientTranslation(direction, donorPos, hydrogenPos, acceptorPos, isHSide);
            case AngleBeta: // Angle H-A-AA
                return betaGradientTranslation(direction, hydrogenPos, acceptorPos, parentAcceptorPos, isHSide);
            case Dihedral_1: // Dihedral C-D-H-A
                return dihedral1GradientTranslation(direction, parentDonorPos, donorPos, hydrogenPos, acceptorPos, isHSide);
            case Dihedral_2: // Dihedral D-H-A-AA
                return dihedral2GradientTranslation(direction, donorPos, hydrogenPos, acceptorPos, parentAcceptorPos, isHSide);
            default:
                return 0.0f;
        }
    }
    else // rotation
    {
        glm::vec3 p_c = dof.atoms->derivedCentre();
        glm::vec3 axisA = p_c;
        glm::vec3 axisB = p_c + direction;
        switch (type)
        {
            case Distance:  // Distance H–A
              return isHSide ?
                    bond_rotation_on_distance_gradient(axisA, axisB, acceptorPos, hydrogenPos) : 
                    bond_rotation_on_distance_gradient(axisA, axisB, hydrogenPos, acceptorPos);
            case AngleAlpha: // Angle D-H-A
                return isHSide ?
                      alphaGradientHSide(axisA, axisB, donorPos, hydrogenPos, acceptorPos, false) : 
                      alphaGradientASide(axisA, axisB, donorPos, hydrogenPos, acceptorPos);
            case AngleBeta: // Angle H-A-AA
                return isHSide ?
                      betaGradientHSide(axisA, axisB, hydrogenPos, acceptorPos, parentAcceptorPos) : 
                      betaGradientASide(axisA, axisB, hydrogenPos, acceptorPos, parentAcceptorPos, false);
            case Dihedral_1: // Dihedral C-D-H-A
                return isHSide ?
                      dihedral1GradientHSide(axisA, axisB, parentDonorPos, donorPos, hydrogenPos, acceptorPos, false) : 
                      dihedral1GradientASide(axisA, axisB, parentDonorPos, donorPos, hydrogenPos, acceptorPos);
            case Dihedral_2: // Dihedral D-H-A-AA
                return isHSide ?
                  dihedral2GradientHSide(axisA, axisB, donorPos, hydrogenPos, acceptorPos, parentAcceptorPos) : 
                  dihedral2GradientASide(axisA, axisB, donorPos, hydrogenPos, acceptorPos, parentAcceptorPos, false);
            default:
                return 0.0f;
        }

    }
    return 0.0f;
}

BondEntity::AxisAndPositions BondEntity::computeAxisAndPositions(int pivotBlockIdx, 
                                                        const std::vector<AtomBlock> &blocks) const
{
    AxisAndPositions p;
    p.axisA = blocks[pivotBlockIdx].my_position();
    int parentIdx = pivotBlockIdx + blocks[pivotBlockIdx].parent_idx;
    p.axisB = blocks[parentIdx].my_position();
    p.donorPos = blocks[donorIdx].my_position();
    p.acceptorPos = blocks[acceptorIdx].my_position();
    return p;
}


void Flexibility::newJacobian()
{
    // COLUMNS = CONSTRAINTS
    // ROWS = DOFS
    if (_hbonds.size() == 0) 
    {
        std::cerr << "Error: No HBonds to add." << std::endl;
        return;
    }

    int numRow = _activeDoFMap.size(); //replace dofMap.size
    int numCol = _constraintMap.size(); // vdw already included in the constraint map 
    Eigen::MatrixXf jacobianMatrix(numRow, numCol);
    jacobianMatrix.setZero();

    const std::vector<AtomBlock> &blocks = _resources.sequences->sequence()->blocks();

    for (auto &[row, dof] : _activeDoFMap) // was _dofMap
    {
        if (dof.type == Torsion)
        {
            OpSet<int> pivotSet = _resources.sequences->sequence()->blocksForTorsionIdx(dof.idx);
            std::vector<int> pivotIndices = pivotSet.toVector();

            for (int pivotBlockIdx : pivotIndices)
            {
                for (auto &[col, constraint] : _constraintMap)
                {
                    if (dof.atoms == constraint.donorGroup ||
                        dof.atoms == constraint.acceptorGroup)
                    {
                        float deriv = constraint.hbond->getDerivative(constraint.type, dof,
                                                                 pivotBlockIdx, blocks);
                        jacobianMatrix(row, col) = deriv;
                    }
                }
            }
        } 
        else // rigid body - no pivot block 
        {
            for (auto &[col, constraint] : _constraintMap)
            {
                if (dof.atoms == constraint.donorGroup ||
                    dof.atoms == constraint.acceptorGroup)
                {
                    float deriv = constraint.hbond->getDerivative(constraint.type, dof,
                                                             -1, blocks);
                    jacobianMatrix(row, col) = deriv;
                }
            }

        }
    }
    _jacobMtx = jacobianMatrix;
    std::cout << "[DEBUG] NEW Jacobian rows (DoF): " << _jacobMtx.rows() << std::endl;
    std::cout << "[DEBUG] NEW Jacobian cols (constraints): " << _jacobMtx.cols() << std::endl;
    std::cout << "[DEBUG]x NEW Jacobian non-zero entries: " 
              << (_jacobMtx.array() != 0.0f).count() << std::endl;

    std::set<AtomGroup*> rbGroupsSeen;
    for (auto &[row, dof] : _activeDoFMap)
        if (dof.type != Torsion) rbGroupsSeen.insert(dof.atoms);

    std::cout << "[DEBUG] Distinct RB groups in _activeDoFMap: " << rbGroupsSeen.size() << std::endl;
    for (auto *g : rbGroupsSeen)
        std::cout << "  group chain: " << g->atomVector()[0]->chain()
                   << " size: " << g->size() << std::endl;

    writeJacobianStatsToCSV("jacobian_stats.csv");
    writeJacobianToCSV("jacobian_new.csv");
}

