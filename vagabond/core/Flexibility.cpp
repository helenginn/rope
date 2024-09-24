#include <vagabond/core/Flexibility.h>
#include <vagabond/utils/maths.h>
#include <vagabond/c4x/Cluster.h>
#include <gemmi/elem.hpp>
#include "Instance.h"
#include "AtomGroup.h"
#include "BondSequence.h"
#include "BondCalculator.h"
#include "BondSequenceHandler.h"
#include <vagabond/core/BondCalculator.h>

// Initializes the Flexibility object with an instance pointer
Flexibility::Flexibility(Instance *i) 
{
	setInstance(i); 
	_instance->load(); 
}

Flexibility::~Flexibility() 
{
	stopGui();
	_instance->unload();
}

// Submits a flexibility calculation job and retrieves the result
float Flexibility::submitJobAndRetrieve(float weight) 
{
	std::cout << "weight: " << weight << std::endl;	

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
	CoordManager* that_is_manager = _resources.sequences->manager(); // Gets the coordinate manager
	calculateTorsionFlexibility(that_is_manager); // Calculates torsion flexibility
}

// Calculates torsion flexibility using a lambda function
void Flexibility::calculateTorsionFlexibility(CoordManager* specific_manager) 
{
	auto calculateFlexibility = [](const Coord::Get &get, const int &idx) 
	{
		float torsion = get(0); // Gets torsion value
		return torsion; 
	};
	specific_manager->setTorsionFetcher(calculateFlexibility); // Sets the torsion fetcher
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


void Flexibility::addHBond(std::string donor, std::string acceptor)
{
	AtomGroup* atomGroup = _instance->currentAtoms();
	Atom* donorAtom = atomGroup->atomByDesc(donor);
	Atom* acceptorAtom = atomGroup->atomByDesc(acceptor);

   // Check if either atom is null
    if (donorAtom == nullptr || acceptorAtom == nullptr)
    {
        std::cerr << "Error: Could not find one or both atoms in the AtomGroup." << std::endl;
        return;
    }

    // calculate HBond distance
    int donorBlock_idx = accessAtomBlock(donorAtom);
    int acceptorBlock_idx = accessAtomBlock(acceptorAtom);

    // Access the donor and acceptor AtomBlock objects
    const std::vector<AtomBlock>& blocks = _resources.sequences->sequence()->blocks();
    const AtomBlock& donorBlock = blocks[donorBlock_idx];
    donorBlock.printBlock();
    const AtomBlock& acceptorBlock = blocks[acceptorBlock_idx];

    glm::vec3 donorPos = blocks[donorBlock_idx].my_position(); 
   	glm::vec3 acceptorPos = blocks[acceptorBlock_idx].my_position();

   	glm::vec3 parentDonorPos = blocks[donorBlock_idx-1].atom->initialPosition(); // replace with DonorBlock.parent_idx 
    glm::vec3 parentAcceptorPos = blocks[acceptorBlock_idx-1].atom->initialPosition();

    // Calculate alpha-vectors
    glm::vec3 alphaVector1 = parentDonorPos - donorPos;
    glm::vec3 alphaVector2 = acceptorPos - donorPos;

    float distance = calculateDistance(donorPos, acceptorPos);
    float alphaAngle = calculateAngle(alphaVector1, alphaVector2);

	// Create HBondEntity and store values
    HBondEntity hbe;
    hbe.Donor = donorAtom;
    hbe.Acceptor = acceptorAtom;
    hbe.startDist = distance;
    hbe.ParentDonor = blocks[donorBlock_idx - 1].atom;
    hbe.ParentAcceptor = blocks[acceptorBlock_idx - 1].atom;
    hbe.AlphaAngle = alphaAngle;

    // Push back the HBondEntity into the _hbonds vector
    _hbonds.push_back(hbe);


    // Print the positions and distance for debugging
    std::cout << "Donor position: " << donorPos.x << ", " << donorPos.y << ", " << donorPos.z << std::endl;
    std::cout << "Acceptor position: " << acceptorPos.x << ", " << acceptorPos.y << ", " << acceptorPos.z << std::endl;
    std::cout << "Distance: " << distance << std::endl;
    std::cout << "Parent donor atom position: " << parentDonorPos.x << ", " << parentDonorPos.y << ", " << parentDonorPos.z << std::endl;
    std::cout << "Parent acceptor atom position: " << parentAcceptorPos.x << ", " << parentAcceptorPos.y << ", " << parentAcceptorPos.z << std::endl;
    std::cout << "Angle alpha (degrees): " << alphaAngle << " degrees" << std::endl;

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
            std::cout << "Atom " << atom->desc() << " is in block " << i << std::endl;
            return i; // Return the index of the block
        }
    }

    // Atom not found in any block
    return -1;
}


void Flexibility::printHBonds() const
{
    std::cout << "HBond Entities:" << std::endl;
    for (const HBondEntity& hbe : _hbonds) 
    {
        std::cout << "  Donor: " << hbe.Donor->desc() << std::endl;
        std::cout << "  Acceptor: " << hbe.Acceptor->desc() << std::endl;
        std::cout << "  Start Distance: " << hbe.startDist << std::endl;
        std::cout << "  Parent Donor: " << hbe.ParentDonor->desc() << std::endl;
        std::cout << "  Parent Acceptor: " << hbe.ParentAcceptor->desc() << std::endl;
        std::cout << "  Alpha angle: " << hbe.AlphaAngle << std::endl;
        std::cout << "--------------------" << std::endl;
    }
}











