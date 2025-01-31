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

using Eigen::MatrixXf;
using Eigen::VectorXf;
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

// Calculates torsion flexibility using a lambda function
void Flexibility::calculateTorsionFlexibility(CoordManager* specific_manager) 
{
    std::cout << "Starting caclulating torsion Flexibility" << std::endl;
	auto calculateFlexibility = [this](const Coord::Get &get, const int &idx) 
	{
		float weight = get(0); // Gets weight value passed into submitJob
        int bigVecIdx = idx; 
        float flexWeight = _allTorsions[bigVecIdx]*weight;
		return flexWeight; 
	};
	specific_manager->setTorsionFetcher(calculateFlexibility); // Sets the torsion fetcher
    std::cout << "Finishing caclulating torsion Flexibility" << std::endl;
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


void Flexibility::addMultipleHBonds(const std::vector<HBondManager::HBondPair> &donorAcceptorPairs) 
{

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
    CoordManager* that_is_manager = _resources.sequences->manager(); 
    // Calculates torsion flexibility
    calculateTorsionFlexibility(that_is_manager); 
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


void Flexibility::addHBond(const HBondManager::HBondPair &hbondPair) {
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

            // std::cout << "derivatives " << numRow << std::endl;
            // std::cout << "Processing row " << i << ", column " << j << std::endl;
            // std::cout << "APos: " << APos.x << ", " << APos.y << ", " << APos.z << std::endl;
            // std::cout << "BPos: " << BPos.x << ", " << BPos.y << ", " << BPos.z << std::endl;
            // std::cout << "CPos: " << CPos.x << ", " << CPos.y << ", " << CPos.z << std::endl;
            // std::cout << "DPos: " << DPos.x << ", " << DPos.y << ", " << DPos.z << std::endl;

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
    std::cout << "Singular values: " << svdJac.singularValues().transpose() << std::endl;

    svdJac.compute(jacobMtrT, Eigen::ComputeFullU | Eigen::ComputeFullV);
    _U = svdJac.matrixU();
    _singularValues = svdJac.singularValues();
    _V = svdJac.matrixV();

}


void Flexibility::calculateFlexWeights()
{
    std::cout << "Calculating flex weights..." << std::endl;

    // Get all the torsions of the protein
    int totalTorsionNum = _resources.sequences->torsionBasis()->parameterCount();
    std::cout << "Total torsions: " << totalTorsionNum << std::endl;

    std::vector<float> weightColumn(_V.rows());
    std::cout << "Weight column size: " << _V.rows() << std::endl;

    for (int i = 0; i < _V.rows(); ++i)
    {
        weightColumn[i] = _V(i, _V.cols() - 1);
    }

    _allTorsions = std::vector<float>(totalTorsionNum); // Initialize with zeros
    std::cout << "_allTorsions size: " << _allTorsions.size() << std::endl;

    // Debug size of _globalTorsionVector
    std::cout << "_globalTorsionVector size: " << _globalTorsionVector.size() << std::endl;

    if (_globalTorsionVector.size() != weightColumn.size())
    {
        std::cerr << "Error: Size mismatch between _globalTorsionVector ("
                  << _globalTorsionVector.size() << ") and weightColumn ("
                  << weightColumn.size() << ")." << std::endl;
        return;
    }

    // Assign weights to torsion vectors
    for (int i = 0; i < _globalTorsionVector.size(); ++i)
    {
        int index = _globalTorsionVector[i];
        if (index < 0 || index >= totalTorsionNum)
        {
            std::cerr << "Error: Index out of bounds in _globalTorsionVector: "
                      << index << std::endl;
            continue;
        }
        _allTorsions[index] = weightColumn[i];
    }

    std::cout << "Finished calculating flex weights." << std::endl;
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






