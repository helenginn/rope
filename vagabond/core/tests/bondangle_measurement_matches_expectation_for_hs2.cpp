#include "../CifFile.h"
#include "../AtomGroup.h"
#include "../BondAngle.h"
#include "../BondLength.h"
#include "../BondCalculator.h"

int main()
{
	std::string path = "/assets/geometry/H2S.cif";

	CifFile geom = CifFile(path);
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
	result->destroy();

	calculator.finish();

	Atom *s = atoms->firstAtomWithName("S");
	Atom *hs1 = atoms->firstAtomWithName("HS1");
	Atom *hs2 = atoms->firstAtomWithName("HS2");
	
	glm::vec3 sp   = s->derivedPosition();
	glm::vec3 hs1p = hs1->derivedPosition();
	glm::vec3 hs2p = hs2->derivedPosition();
	
	float angle = glm::angle(normalize(hs1p - sp),
	                         normalize(hs2p - sp));

	BondAngle *bondAngle = s->bondAngle(0);
	float observed = bondAngle->measurement();

	if (fabs(rad2deg(angle) - observed) > 1e-3)
	{
		std::cout << "Angles do not match for " << bondAngle->desc() << ": " 
		<< rad2deg(angle) << " vs "
		<< observed << std::endl;
		std::cout << glm::to_string(sp) << std::endl;
		std::cout << glm::to_string(hs1p) << std::endl;
		std::cout << glm::to_string(hs2p) << std::endl;
		return 1;
	}
	
	delete atoms;
	
	return 0;
}

