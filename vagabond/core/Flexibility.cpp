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

void Flexibility::generateAtomCloud()
{
    setFlexTag("flexPos");
    const AtomVector &atoms = _instance->currentAtoms()->atomVector();

    for (Atom *atom : atoms)
    {
        atom->removeOtherPosition(_flexTag); 
    }

    // for (float weight = -0.5; weight <= 0.5001; weight += 0.1)
    std::vector<float> weights = { -3.0f, 0.0f, 3.0f };
    for (float weight : weights)
    {
        atomCloud(weight, atoms);
    }

    std::cout << "Samples in first atom: "
          << atoms.front()->otherPositions(_flexTag).samples.size()
          << std::endl;

    std::cout << "End of B-factor estimation!" << std::endl;
    std::cout << "*** Saving position to sampled_positions.csv... ***" << std::endl;
    savePositionsToCSV("sampled_positions.csv", _flexTag, atoms);
    std::cout << "Average positions" << std::endl;
    calculateAnisoBfactors(_flexTag, atoms);
    saveBfactorsToCSV("bfactors.csv", _flexTag, atoms);
}



void Flexibility::atomCloud(float weight, const AtomVector &atoms)
{
    // change here witht the values that you get from the FlexView (give by the user)
    for (int i = _minCol; i <= _maxCol; ++i)
    {
        _colIdx = i;
        submitJobAndRetrieve(weight);
        for (Atom *atom : atoms)
        {
            glm::vec3 vec = atom->derivedPosition();
            atom->addOtherPosition(_flexTag, vec);   
        }
    }

}

void Flexibility::savePositionsToCSV(const std::string &filename, std::string &_flexTag, const AtomVector &atoms)
{
    std::ofstream file(filename);
    if (!file.is_open())
    {
        std::cerr << "Could not open file: " << filename << std::endl;
        return;
    }

    // Write header
    file << "Description,Element,ResidueID,AtomName,Chain,SampleIndex,X,Y,Z\n";
    for (Atom *atom : atoms)
        {
            const WithPos &positions = atom->otherPositions(_flexTag);
            for (size_t i = 0; i < positions.samples.size(); ++i)
            {
                const glm::vec3 &pos = positions.samples[i];

                file << atom->desc() << ","
                     << atom->elementSymbol() << ","
                     << atom->residueId() << ","
                     << atom->atomName() << ","
                     << atom->chain() << ","
                     << i << ","
                     << std::fixed << std::setprecision(3)
                     << pos.x << ","
                     << pos.y << ","
                     << pos.z << "\n";
            }
        }

    file.close();
    std::cout << "Saved sampled positions to " << filename << std::endl;
}

void Flexibility::saveBfactorsToCSV(const std::string &filename, std::string &_flexTag, const AtomVector &atoms)
{
    std::ofstream file(filename);
    if (!file.is_open())
    {
        std::cerr << "Could not open file: " << filename << std::endl;
        return;
    }

    // Write header
    file << "Description,Element,ResidueID,AtomName,Chain,"
         << "B11,B22,B33,B12,B13,B23\n";
    for (Atom *atom : atoms)
        {
            const Eigen::Matrix3f &cov = atom->otherAnisoBfactors(_flexTag);
            file << atom->desc() << ","
                 << atom->elementSymbol() << ","
                 << atom->residueId() << ","
                 << atom->atomName() << ","
                 << atom->chain() << ","
                 << std::fixed << std::setprecision(4)
                 << cov(0, 0) << "," << cov(1, 1) << "," << cov(2, 2) << ","
                 << cov(0, 1) << "," << cov(0, 2) << "," << cov(1, 2) << "\n";

        }

    file.close();
    std::cout << "Saved Bfactors to " << filename << std::endl;
}

void Flexibility::calculateAnisoBfactors(std::string &_flexTag, const AtomVector &atoms)
{

    for (Atom *atom : atoms)
        {
            const WithPos &positions = atom->otherPositions(_flexTag);
            const std::vector<glm::vec3> &samples = positions.samples;
            if (samples.empty())
            {
                continue;
            }
            glm::vec3 sum(0.f);
            for (const glm::vec3 &pos : samples)
            {
                sum += pos;
            }
            glm::vec3 avg = sum / static_cast<float>(samples.size());
            atom->setDerivedPosition(avg);
            // calculate covariance matrix 
            Eigen::Matrix3f covMat = covariance(samples);
            // Output: average and covariance
            std::cout << atom->desc() << ","
                    << atom->elementSymbol() << ","
                    << atom->residueId() << ","
                    << atom->atomName() << ","
                    << atom->chain() << ","
                    << std::fixed << std::setprecision(3)
                    << "AVG: (" << avg.x << ", " << avg.y << ", " << avg.z << "), " << std::endl;
            Eigen::IOFormat cleanFmt(3, 0, ", ", "\n", "[", "]");
            std::cout << "COV:\n" << covMat.format(cleanFmt) << "\n";
            // Regularize the covariance matrix
            float epsilon = 0.01f; // in Å²
            // covMat += epsilon * Eigen::Matrix3f::Identity();
            Eigen::Matrix3f bFactorTens = 8 * M_PI * M_PI * covMat;
            std::cout << "B factor tenstor:\n" << bFactorTens.format(cleanFmt) << "\n";
            atom->setDerivedAnisoBfactors(bFactorTens);

        }

}


Eigen::Matrix3f Flexibility::covariance(const std::vector<glm::vec3> &samples)
{
    glm::vec3 mean(0.0f);
    for (const glm::vec3 &v : samples)
    {
        mean += v;
    }
    mean /= static_cast<float>(samples.size());

    Eigen::Matrix3f cov = Eigen::Matrix3f::Zero();
    for (const glm::vec3 &v : samples)
    {
        Eigen::Vector3f diff(v.x - mean.x, v.y - mean.y, v.z - mean.z);
        cov += diff * diff.transpose();
    }

    cov /= static_cast<float>(samples.size());
    return cov;
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
        int access_idx = _allTorsions.size() - _colIdx;
        float val = _allTorsions[_colIdx][idx] * jobWeight;
        return val;
    };

    coord_manager->setTorsionFetcher(calculateFlexibility);

    std::cout << "Finished calculating torsion Flexibility" << std::endl;
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
    
    int parentDonor_idx = blocks[donorBlock_idx].parent_idx;
    int parentAcceptor_idx = blocks[acceptorBlock_idx].parent_idx;

    // Compute positions, distances, and angles
    glm::vec3 donorPos = blocks[donorBlock_idx].my_position();
    glm::vec3 acceptorPos = blocks[acceptorBlock_idx].my_position();
    glm::vec3 parentDonorPos = blocks[donorBlock_idx + parentDonor_idx].my_position();
    glm::vec3 parentAcceptorPos = blocks[acceptorBlock_idx + parentAcceptor_idx].my_position();

    float distance = calculateDistance(donorPos, acceptorPos);
    float alphaAngleDistance = calculateAngleDistance(donorPos, acceptorPos, parentDonorPos);
    float betaAngleDistance = calculateAngleDistance(acceptorPos, donorPos, parentAcceptorPos);

    // Create HBondEntity and store values
    HBondEntity hbe;
    hbe.Donor = donorAtom;
    hbe.donorIdx = donorBlock_idx;
    hbe.Acceptor = acceptorAtom;
    hbe.acceptorIdx = acceptorBlock_idx;
    hbe.Hydrogen = hydrogenAtom;                
    hbe.hydrogenIdx = hydrogenBlock_idx;
    hbe.startDist = distance;
    hbe.ParentDonor = blocks[donorBlock_idx - 1].atom;
    hbe.ParentAcceptor = blocks[acceptorBlock_idx - 1].atom;
    hbe.AlphaAngleDist = alphaAngleDistance;
    hbe.BetaAngleDist = betaAngleDistance;

    std::vector<int> lca_idx = lastCommonAncestorIdx(donorBlock_idx, acceptorBlock_idx);
    // Insert torsion vector to _hbe 
    hbe.TorsionVec = lca_idx;
    _hbonds.push_back(hbe);
    _globalTorsionSet.insert(hbe.TorsionVec.begin(), hbe.TorsionVec.end());
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
            Atom *atom_j = atoms[j];
            int block_j = accessAtomBlock(atom_j);
            double r_j = getVdWRadius(atom_j);
            if (r_j <= 0.0) continue;
            glm::vec3 pos_j = blocks[block_j].my_position();
            
            // calculate distance between the two atoms
            glm::vec3 diff = pos_i - pos_j;
            float dist_sq = glm::dot(diff, diff);

            double threshold = r_i + r_j + cutoffD;

            if (dist_sq < threshold*threshold)
            {
                VdWBondEntity vdw;
                vdw.Atom1 = atom_i;
                vdw.atomIdx1 = block_i;
                vdw.Atom2 = atom_j;
                vdw.atomIdx2 = block_j;
                vdw.startDist = glm::length(diff);
                vdw.contactDist =  threshold;
                vdw.TorsionVec = lastCommonAncestorIdx(block_i, block_j);
                _VdWBonds.push_back(vdw);
            }
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


std::vector<int> Flexibility::lastCommonAncestorIdx(int donorBlock_idx, int acceptorBlock_idx)
{
    std::vector<int> torsionVector;
    const std::vector<AtomBlock>& blocks = _resources.sequences->sequence()->blocks();
    while (true)
    {
        if (blocks[donorBlock_idx].depth > blocks[acceptorBlock_idx].depth)
        {
            donorBlock_idx = rewindBlock(donorBlock_idx, torsionVector);
        }
        else if (blocks[donorBlock_idx].depth < blocks[acceptorBlock_idx].depth)
        {
            acceptorBlock_idx = rewindBlock(acceptorBlock_idx, torsionVector);  
        }
        else // equal depth 
        {
            if (blocks[donorBlock_idx].depth == blocks[acceptorBlock_idx].depth) // found common ancestor 
            {
                return torsionVector;
            }
            // rewind at the same time
            acceptorBlock_idx = rewindBlock(acceptorBlock_idx, torsionVector);
            donorBlock_idx = rewindBlock(donorBlock_idx, torsionVector);
        }

    }
}

int Flexibility::rewindBlock(int &block_idx, std::vector<int> &torsionVector) 
{
    const std::vector<AtomBlock>& blocks = _resources.sequences->sequence()->blocks();
    TorsionBasis *basis = _resources.sequences->sequence()->torsionBasis();
    int blockParent_idx = blocks[block_idx].parent_idx;
    block_idx += blockParent_idx;

    // check for peptide bonds   
    // If the block has torsion larger than -1, add them to the torsion vector
    if (blocks[block_idx].torsion_idx >= 0) 
    {
        Parameter *p = basis->parameter(blocks[block_idx].torsion_idx);
        if (p->isPeptideBond())
        {
            return block_idx; // Do NOT push on torsionVector, but update block_idx
        }
        torsionVector.push_back(blocks[block_idx].torsion_idx); // if the hbond is between two molecules htat are 
                                                                //not conected with a common ancestor: this case                                                                // need to be handle this case
    }
    return block_idx;
}


void Flexibility::buildJacobianMatrix()
{
    if (_hbonds.size() == 0) 
    {
        std::cerr << "Error: No HBonds to add." << std::endl;
        return;
    }
    // Columns = total number of constraints (5 per Hbonds +one per vdW bonds)
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
        int torsionIdx = torsionVector[i];
        int colBase = 0;
        for (int j = 0; j < _hbonds.size(); ++j) 
        {
            HBondEntity& hbe = _hbonds[j];
            // int colBase = j * 5; // 5 constraints per Hbond

            glm::vec3 APos = blocks[torsionIdx].my_position(); 
            glm::vec3 BPos = blocks[torsionIdx].inherit; 
            
            glm::vec3 CPos = blocks[hbe.acceptorIdx].my_position(); 
            glm::vec3 DPos = blocks[hbe.donorIdx].my_position(); 
            glm::vec3 HPos = blocks[hbe.hydrogenIdx].my_position();

            int parentDonor_idx = blocks[hbe.donorIdx].parent_idx;
            int parentAcceptor_idx = blocks[hbe.acceptorIdx].parent_idx;

            glm::vec3 parentDonor = blocks[hbe.donorIdx + parentDonor_idx].my_position();
            glm::vec3 parentAcceptor = blocks[hbe.acceptorIdx + parentAcceptor_idx].my_position();

            // 1) Distance H–A
            float dDist = bond_rotation_on_distance_gradient(APos, BPos, CPos, DPos);
            jacobianMatrix(i,colBase + 0) = dDist;

            // 2) Angle D-H-A
            float dAngle1 = bond_rotation_on_angle_gradient(APos, BPos, DPos, HPos, CPos);
            jacobianMatrix(i,colBase + 1) = dAngle1;

            // 3) Angle H-A-B
            float dAngle2 = bond_rotation_on_angle_gradient(APos, BPos,
                                                HPos, CPos, parentAcceptor);
            jacobianMatrix(i,colBase + 2) = dAngle2;

            // 4) Dihedral C-D-H-A
            float dDihedral1 = bond_rotation_on_torsion_gradient(APos, BPos, parentDonor, DPos, HPos, CPos);
            jacobianMatrix(i, colBase + 3) = dDihedral1;

            // 4) Dihedral D-H-A-B
            float dDihedral2 = bond_rotation_on_torsion_gradient(APos, BPos, DPos, HPos, CPos, parentAcceptor);
            jacobianMatrix(i, colBase + 4) = dDihedral2;
            colBase += 5;

        }
    }

    // --- VdW bonds ---
    int vdwColBase = 5 * _hbonds.size();
    for (int i = 0; i < numRow; ++i) 
    {
        int torsionIdx = torsionVector[i];
        for (int j = 0; j < _VdWBonds.size(); ++j) 
        {
            VdWBondEntity& vdw = _VdWBonds[j];

            glm::vec3 APos = blocks[torsionIdx].my_position(); 
            glm::vec3 BPos = blocks[torsionIdx].inherit; 
            glm::vec3 CPos = blocks[vdw.atomIdx1].my_position(); 
            glm::vec3 DPos = blocks[vdw.atomIdx2].my_position(); 

            float derivative = bond_rotation_on_distance_gradient(APos, BPos, CPos, DPos);
            jacobianMatrix(i, vdwColBase + j) = derivative;
        }
    }

    _jacobMtx = jacobianMatrix;
}

SVDResult Flexibility::calculateSVD() const
{
    MatrixXf jacobMtrT = _jacobMtx.transpose();
    BDCSVD<MatrixXf> svd(jacobMtrT, Eigen::ComputeFullU | Eigen::ComputeFullV);


// debuging 
    Eigen::IOFormat CleanFmt(4, 0, ", ", "\n", "[", "]");

    std::cout << "[DEBUG]" << std::endl;
    std::cout << "\n================= JACOBIAN MATRIX =================\n";
    std::cout << "Dimensions: " << _jacobMtx.rows() << " x " << _jacobMtx.cols() << "\n";
    std::cout << _jacobMtx.format(CleanFmt) << "\n";

    std::cout << "\n================= SINGULAR VALUES =================\n";
    std::cout << svd.singularValues().transpose().format(CleanFmt) << "\n";

    std::cout << "\n================= V MATRIX (Null-space basis) =================\n";
    std::cout << "Dimensions: " << svd.matrixV().rows() << " x " << svd.matrixV().cols() << "\n";
    std::cout << svd.matrixV().format(CleanFmt) << "\n";

    std::cout << "\n===================================================\n";


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
    _vSize = static_cast<int>(svd.V.cols());
    _V = svd.V;
    _S = svd.singularValues;
    std::vector<int> torsionVector = getGlobalTorsionVector();
    int totalTorsionNum = _resources.sequences->torsionBasis()->parameterCount();
    if (torsionVector.size() != svd.V.rows())
    {
        std::cerr << "Error: Size mismatch between globalTorsionVector ("
                  << torsionVector.size() << ") and V rows ("
                  << svd.V.rows() << ")." << std::endl;
        return;
    }
    int maxCol = _useSingleColumn ? _colIdx + 1 : std::min(_colIdx + 1, _vSize);
    for (int colIdx = 0; colIdx < _vSize; ++colIdx)
    {
        std::vector<float> v_i = extractVColumn(svd.V, colIdx);
    
        std::vector<float> allTorsions = assignWeightsToTorsions(v_i, torsionVector);
        _allTorsions.push_back(allTorsions);
    }
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

std::vector<float> Flexibility::assignWeightsToTorsions(const std::vector<float>& v_i,
                                          const std::vector<int>& torsionVector)
{
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


std::vector<int> Flexibility::sampleColumnIndices(int N, int sampleCount, double lambda)
{
    std::vector<double> weights(N);
    for (int i=0; i<N; i++)
    {
        weights[i] = std::exp(lambda * i);
    }
    // normalise weights 
    double sum = std::accumulate(weights.begin(), weights.end(), 0.0);
    for (auto& w : weights) w /= sum;

    // compute CDF
    std::vector<double> cdf(N);
    cdf[0] = weights[0];
    for (int i = 1; i < N; ++i)
        cdf[i] = cdf[i - 1] + weights[i];
    // Sample indices
    std::vector<int> sampled;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 1.0);
    while (sampled.size() < sampleCount)
    {
        double r = dis(gen);
        auto it = std::lower_bound(cdf.begin(), cdf.end(), r);
        int idx = std::distance(cdf.begin(), it);
        sampled.push_back(idx);
    }
    return sampled;
}

std::vector<int> Flexibility::getSoftestModeIndices(const Eigen::VectorXf& singularValues)
{
    int totalModes = static_cast<int>(singularValues.size());
    std::vector<int> indices(totalModes);
    std::cout << "[getSoftestModeIndices]: Singular values (nonzero only):\n";
    for (int i = totalModes -1; i >= 0; i--)
    {
        if (singularValues[i] < 1e-6)
        {
            std::cout << "σ[" << i << "] = " << singularValues[i] << "\n";
            indices.push_back(i);
        }
    }
    return indices;
}
 

void Flexibility::saveSampledStructures(int numSamples, const std::string& baseFileName, double lambda)
{

    std::cout << "[saveSampledStructures] _S size = " << _S.size() << "\n";
    if (_S.size() > 0)
        std::cout << "First few singular values: " << _S.head(std::min<int>(10, _S.size())).transpose() << "\n";
    else
        std::cerr << "[saveSampledStructures] _S is empty!\n";
    // Call sampleColumnIndices to choose numSamples columns from _V
    // std::vector<int> indices = sampleColumnIndices(_vSize, numSamples, lambda);
    std::vector<int> indices = getSoftestModeIndices(_S);
    // Print the indices and their corresponding singular values

    
    // start - debugging
    const unsigned displayLimit = 5;

    int saved = 0;
    int attempts = 0;
    int maxAttempts = numSamples * 10; // safeguard: don't loop forever

    // prepare radii
    std::set<std::pair<int,int>> exclude;
    std::vector<float> radii;

    const AtomVector &atoms = _instance->currentAtoms()->atomVector();
    OpSet<Atom*> atom_set(atoms);
    std::vector<Atom*> orderedAtoms = atom_set.toVector();
    radii = makeRadiiVec(orderedAtoms);

    // prepare exclude
    exclude = makeExcList(atom_set);

    // * Call submitJob for the randomWeight
    double weight = 0;
    submitJob(weight);
    Result *r = _resources.calculator->acquireObject();
    r->transplantPositions(false, true); // Or true, depending on what you want saved

    while (saved < numSamples && attempts < maxAttempts)
    {
        ++attempts;
        // * store index in _colIdx
        // try:
        _colIdx = indices[attempts % indices.size()];
        // _colIdx = indices[saved];

        // * Generate a random scalar weight in [-10, 10] (amplitude of the pertubation along that column)
        std::random_device rd; std::mt19937 gen(rd()); 
        std::uniform_real_distribution<> dis(-5.0, 5.0); 
        double randomWeight = dis(gen);
        // * Call submitJob for the randomWeight
        submitJob(randomWeight);
        Result *r = _resources.calculator->acquireObject();
        r->transplantPositions(false); // Or true, depending on what you want saved


        // check for classes
        float tol = 0.25f;
        bool clashOK = checkClashes(orderedAtoms, saved, radii, exclude, tol);
        if (!clashOK)
        {
            std::cerr << "[saveSampledStructures] Sample " << saved << " rejected due to atom clash\n"; 
            r->destroy(); 
            continue; // skip writing this structure
        }
        std::ostringstream oss; oss << baseFileName << "_" << saved << ".pdb"; 
        _instance->currentAtoms()->writeToFile(oss.str()); 
        r->destroy(); 
        ++saved;   
    }
    if (saved < numSamples) 
    { 
        std::cerr << "[saveSampledStructures] Warning: only saved " 
        << saved << "/" << numSamples << " due to clashes (maxAttempts=" 
        << maxAttempts << ")\n"; 
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
            // std::cout << "[checkClashes] Clash between atoms "
            //               << i << " and " << j << std::endl;

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


void Flexibility::calculateFreeEnergy()
{
// check if svd has already been calculated adn _Vsize and _V has been assinged: 
// this is done in calculateFlexWeights, called in processMultipleHBonds


    if (_V.size() == 0 || _S.size() == 0)
    {
        std::cerr << "Error: SVD has not been computed. "
                  << "Please run calculateFlexWeights() or equivalent first." 
                  << std::endl;
    }
    std::cout << "Calculating free energies for " << _vSize << " modes..." << std::endl;

    int numModes = _S.size();;
    std::vector<double> enthalpies(numModes);
    std::vector<double> entropies(numModes);
    std::vector<double> freeEnergies(numModes);

    for (int i = 0; i < numModes; ++i)
    {
        // pick up singular values for matrix _S:
        double sigma_i = _S(i);
        double enthalpy = computeEnthalpy(sigma_i);
        enthalpies[i] = enthalpy;

        std::vector<float> v_i = extractVColumn(_V, i);
        double entropy = computeEntropy(v_i);
        entropies[i] = entropy;

        // calucate free energy
        double c_T = 1.0;
        freeEnergies[i] = enthalpy - c_T * entropy;
    }
    saveFreeEnergyCSV("enthalpy_entropy_energy.csv", enthalpies, entropies, freeEnergies);
}

double Flexibility::computeEnthalpy(double sigma)
{
    const double k = 3.24; // kcal/mol scaling factor
    double maxSingVal = _S.maxCoeff();
    return (maxSingVal > 0.0) ? sigma / maxSingVal : 0.0;
}

double Flexibility::computeEntropy(const std::vector<float>& v_i)
{
    double sqSum = 0.0;
    std::vector<double> kappa(v_i.size());
    for (size_t j = 0; j < v_i.size(); j++)
    {
        kappa[j] = v_i[j] * v_i[j];
        sqSum += kappa[j];
    }

    if (sqSum > 0)
    {
        for (size_t j = 0; j < kappa.size(); j++)
        {
            kappa[j] /= sqSum;
        }
    }

    // Shannon entropy
    double eVal = 0.0;
    for (size_t j = 0; j < kappa.size(); j++)
    {
        if (kappa[j] > 0)
        {
            eVal += -kappa[j] * log(kappa[j]);
        }
    }

    // normalized collectivity measure: between [1/N, 1]
    return (1.0 / (double)kappa.size()) * exp(eVal);

}

void Flexibility::saveFreeEnergyCSV(const std::string &filename,
                                    const std::vector<double> &enthalpies,
                                    const std::vector<double> &entropies,
                                    const std::vector<double> &freeEnergies)
{
    if (enthalpies.size() != entropies.size() || enthalpies.size() != freeEnergies.size())
    {
        std::cerr << "Error: Mismatched vector sizes for free energy data." << std::endl;
        return;
    }

    std::ofstream file(filename);
    if (!file.is_open())
    {
        std::cerr << "Error: Could not open file " << filename << " for writing." << std::endl;
        return;
    }

    // Write header
    file << "Mode,Enthalpy,Entropy,FreeEnergy\n";

    // Write values with 6 decimal places
    file << std::fixed << std::setprecision(6);
    for (size_t i = 0; i < enthalpies.size(); ++i)
    {
        file << i << "," 
             << enthalpies[i] << "," 
             << entropies[i] << "," 
             << freeEnergies[i] << "\n";
    }

    file.close();
    std::cout << "Free energy data saved to " << filename << std::endl;
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
