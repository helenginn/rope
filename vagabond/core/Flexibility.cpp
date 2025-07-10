#include <vagabond/core/Flexibility.h>
#include <vagabond/utils/maths.h>
#include <vagabond/c4x/Cluster.h>
#include <mutex>
#include <gemmi/elem.hpp>
#include "Instance.h"
#include "AtomGroup.h"
#include "BondSequence.h"
#include "BondCalculator.h"
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
	stopGui();
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

void Flexibility::generateAtomCloud()
{
    setFlexTag("flexPos");
    const AtomVector &atoms = _instance->currentAtoms()->atomVector();

    for (Atom *atom : atoms)
    {
        atom->removeOtherPosition(_flexTag); 
    }

    for (float weight = -1.0f; weight <= 1.0f; weight += 0.1f)
    {
        atomCloud(weight, atoms);
    }

    std::cout << "End of B-factor estimation!" << std::endl;
    std::cout << "*** Saving position to sampled_positions.csv... ***" << std::endl;
    savePositionsToCSV("sampled_positions.csv", _flexTag, atoms);
    std::cout << "Average positions" << std::endl;
    calculateAnisoBfactors(_flexTag, atoms);
    saveBfactorsToCSV("bfactors.csv", _flexTag, atoms);
}

void Flexibility::atomCloud(float weight, const AtomVector &atoms)
{
    for (int i = 0; i <= 5; ++i) // the range here suggests how many columns out of the V matrix you choosing
    {
        setColRange(i); // this will set th private variable _colIdx to certain number
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
            _directCov = calculateCovSVD(covMat);
            // Regularize the covariance matrix
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

Eigen::Matrix3f Flexibility::calculateCovSVD(Eigen::Matrix3f covMtx)
{
    Eigen::MatrixXf covDynamic = covMtx;
    Eigen::BDCSVD<Eigen::MatrixXf> svdCov;
    svdCov.compute(covDynamic, Eigen::ComputeFullU | Eigen::ComputeFullV);

    Eigen::MatrixXf U = svdCov.matrixU();
    Eigen::VectorXf S = svdCov.singularValues();
    Eigen::MatrixXf V = svdCov.matrixV();
    Eigen::IOFormat cleanFmt(3, 0, ", ", "\n", "[", "]");
    std::cout << "singularValues of COV:\n" << S.format(cleanFmt) << "\n";

    // Reconstruct the covariance matrix from SVD
    Eigen::Matrix3f _directCov = U * S.asDiagonal() * V;
    std::cout << "Reconstructed COV from SVD:\n" << _directCov.format(cleanFmt) << "\n";

    return _directCov;
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

void Flexibility::calculateTorsionFlexibility(CoordManager* manager) 
{
    std::cout << "Starting calculating torsion Flexibility" << std::endl;
    auto calculateFlexibility = [this](const Coord::Get &get, const int &idx)
    {
        float jobWeight = get(0);
        // std::cout << "--- Setting torsion fetcher for colIdx = " << _colIdx << " ---" << std::endl;
        return _allTorsionsHistory[_colIdx][idx]*jobWeight;
    };
    manager->setTorsionFetcher(calculateFlexibility);



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
  BondSequence* firstSequence = sequences->getSequences()[0];
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
    
    std::cout << "Successfully loaded HBond pairs into Flexibility." << std::endl;
}


void Flexibility::addMultipleHBonds(const std::vector<HBondManager::HBondPair> &donorAcceptorPairs) // called directly 
{
/**
 * @brief Adds multiple hydrogen bonds (HBonds) and updates flexibility calculations.
 *
 * This function takes a list of all the donor-acceptor hydrogen bond pairs that were foudn in the
 * file, validates them, and integrates them into the system. If the input list is empty, 
 * an error message is displayed, and the function returns early.
 *
 * After adding the HBonds, the function updates key flexibility-related calculations:
 * - Computes the Jacobian matrix.
 * - Calculates Singular Value Decomposition (SVD) matrices.
 * - Updates the flexibility weights.
 * - Retrieves the coordinate manager and recalculates torsion flexibility.
 *
 */

    if (donorAcceptorPairs.empty()) 
    {
        std::cerr << "Error: No HBonds to add." << std::endl;
        return;
    }

    // Calculate the Jacobian matrix
    buildJacobianMatrix();

    // Print the Jacobian matrix
    std::cout << "Jacobian Matrix:" << std::endl;
    // std::cout << _jacobMtx << std::endl;

    // calculate SVD matrices 
    _globalTorsionVector.assign(_globalTorsionSet.begin(), _globalTorsionSet.end());
    calculateSVD();
    calculateFlexWeights();

    // Gets the coordinate manager
    CoordManager* coord_manager = _resources.sequences->manager(); 
    // Calculates torsion flexibility
    calculateTorsionFlexibility(coord_manager); 
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
    Atom* donorAtom = atomGroup->atomByDesc(hbondPair.donor);
    if (!donorAtom) {
        ++missingDonorCount;
        std::cerr << "Error: Donor atom '" << hbondPair.donor 
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
    std::cout << "Validation successful. Total successful validations: " 
              << successfulValidations << std::endl;

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
    // Proceed with the rest of the addHBond logic as before
    AtomGroup* atomGroup = _instance->currentAtoms();
    Atom* acceptorAtom = atomGroup->atomByDesc(hbondPair.acceptor);
    Atom* hydrogenAtom = atomGroup->atomByDesc(hbondPair.donor);

    // Error handling for acceptor
    if (!checkAndGetAtom(atomGroup, hbondPair.acceptor, acceptorAtom) || 
        !checkAndGetAtom(atomGroup, hbondPair.donor, hydrogenAtom)) {
        return;
    }

    Atom* donorAtom = hydrogenAtom->connectedAtom(0); // Assuming bonded atom is donor
    if (!donorAtom) {
        std::cerr << "Error: Hydrogen atom '" << hbondPair.donor << "' is not connected to any atom." << std::endl;
        return;
    }

    // Access donor and acceptor positions
    int donorBlock_idx = accessAtomBlock(donorAtom);
    int acceptorBlock_idx = accessAtomBlock(acceptorAtom);

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
    const std::vector<AtomBlock>& blocks = _resources.sequences->sequence()->blocks();
    for (int i = 0; i < blocks.size(); i++) 
    {
        const AtomBlock& block = blocks[i];

        if (block.atom == atom) 
        {
            // Atom found within the block
            return i; // Return the index of the block
        }
    }

    // Atom not found in any block
    return -1;
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
    int blockParent_idx = blocks[block_idx].parent_idx;
    block_idx += blockParent_idx;
    // If the block has torsion larger than -1, add them to the torsion vector
    if (blocks[block_idx].torsion_idx >= 0) 
    {
        // torsionVector.push_back(blocks[block_idx].torsion_idx); changed to: 
        torsionVector.push_back(blocks[block_idx].torsion_idx); // if the hbond is between two molecules htat are not conected with a common ancestor: this case need to be handle this case
    }
    // print a statement if there is no common ancestor: if you reach 0 (or maybe 1, but basically the first depth)
    return block_idx;
}

void Flexibility::buildJacobianMatrix()
{
    // Get the number of torsions and values
    int numCol = _hbonds.size();
    std::cout << "Number of columns of JacMat = " << numCol << std::endl;
    std::vector<int> globalTorsionVector(_globalTorsionSet.begin(), _globalTorsionSet.end());
    int numRow = _globalTorsionSet.size();
    std::cout << "Number of rows of JacMat = " << numRow << std::endl;
    // set up the JacobianMatrix
    Eigen::MatrixXf jacobianMatrix(numRow, numCol);
    jacobianMatrix.setZero();
    // Loop through the Jacobian matrix and print elements
    for (int i = 0; i < numRow; ++i) 
    {
        for (int j = 0; j < numCol; ++j) 
        {
            int torsionIdx = globalTorsionVector[i];
            HBondEntity& hbe = _hbonds[j];

            const std::vector<AtomBlock>& blocks = _resources.sequences->sequence()->blocks();
            glm::vec3 APos = blocks[torsionIdx].my_position(); 
            glm::vec3 BPos = blocks[torsionIdx].inherit; 
            glm::vec3 CPos = blocks[hbe.acceptorIdx].my_position(); 
            glm::vec3 DPos = blocks[hbe.donorIdx].my_position(); 

            float derivative = bond_rotation_on_distance_gradient(APos, BPos, CPos, DPos);
            jacobianMatrix(i,j) = derivative;
        }
    }
    _jacobMtx = jacobianMatrix;
}

void Flexibility::calculateSVD() 
{
    MatrixXf jacobMtrT = _jacobMtx.transpose();
    BDCSVD<MatrixXf> svdJac = jacobMtrT.bdcSvd();
    svdJac.compute(jacobMtrT, Eigen::ComputeFullU | Eigen::ComputeFullV);
    _U = svdJac.matrixU();
    _singularValues = svdJac.singularValues();
    _V = svdJac.matrixV();
     std::cout << "nullspace matrix size: " << _V.size() << std::endl;

}


void Flexibility::calculateFlexWeights()
{
    std::cout << "Calculating flex weights..." << std::endl;

    // Get all the torsions of the protein
    int totalTorsionNum = _resources.sequences->torsionBasis()->parameterCount();

    //try
    std::vector<float> weightColumn(_V.rows());
    _allTorsions = std::vector<float>(totalTorsionNum, 0.0f); // Initialize with zeros
    // Debug size of _globalTorsionVector
    if (_globalTorsionVector.size() != weightColumn.size())
    {
        std::cerr << "Error: Size mismatch between _globalTorsionVector ("
                  << _globalTorsionVector.size() << ") and weightColumn ("
                  << weightColumn.size() << ")." << std::endl;
        return;
    }
    // finish try
    
    std::vector<std::vector<float>> V_columns; // To store all v_i vectors

    // Extract columns from _V and store as vectors
    for (int colIdx = 0; colIdx < _V.cols(); ++colIdx)
    {
        std::vector<float> v_i;
        // this can be used to select a specific column of the _V. 
        // could be used instead of colIdx
        int selectedCol = _V.cols() - 1; 

        for (int rowIdx = 0; rowIdx < _V.rows(); ++rowIdx)
        {
            // float value = _V(rowIdx, colIdx);
            float value = _V(rowIdx, selectedCol);
            v_i.push_back(value);
        }
        V_columns.push_back(v_i);

        // Assign weights from this column to _allTorsions
        for (int i = 0; i < _globalTorsionVector.size(); ++i)
        {
            int index = _globalTorsionVector[i];
            if (index < 0 || index >= totalTorsionNum)
            {
                std::cerr << "Error: Index out of bounds in _globalTorsionVector: "
                          << index << std::endl;
                continue;
            }
            _allTorsions[index] = v_i[i];
        }
        _allTorsionsHistory.push_back(_allTorsions);
    }
    std::cout << "V_columns.size()" << std::endl;
    std::cout << V_columns.size() << std::endl;

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






