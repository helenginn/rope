#include "../CifFile.h"
#include "../AtomGroup.h"
#include "../BondAngle.h"
#include "../BondLength.h"
#include "../BondCalculator.h"

int main()
{
	std::string path = "/assets/geometry/CMO.cif";

	CifFile geom = CifFile(path);
	geom.setAutomaticKnot(true);
	geom.parse();
	
	AtomGroup *atoms = geom.atoms();
	Atom *anchor = atoms->possibleAnchor(0);
	
	BondCalculator calculator;
	calculator.setPipelineType(BondCalculator::PipelineAtomPositions);
	calculator.setMaxSimultaneousThreads(1);
	calculator.addAnchorExtension(anchor);
	calculator.setup();

	calculator.start();
	
	Job empty_job{};
	calculator.submitJob(empty_job);

	Result *result = nullptr;
	result = calculator.acquireResult();
	result->transplantPositions();
	delete result;

	calculator.finish();

	Atom *c = atoms->firstAtomWithName("C");
	Atom *o = atoms->firstAtomWithName("O");
	
	glm::vec3 bond = c->derivedPosition() - o->derivedPosition();
	float length = glm::length(bond);

	BondLength *bondLength = c->bondLength(0);
	
	float compare = bondLength->measurement();
	
	if (fabs(length - compare) > 1e-6)
	{
		std::cout << "Lengths do not match: " << length << " vs "
		<< compare << " Angstroms." << std::endl;
		return 1;
	}
	
	delete atoms;
	
	return 0;
}

