#include "../src/Cif2Geometry.h"
#include "../src/AtomGroup.h"
#include "../src/BondAngle.h"
#include "../src/BondCalculator.h"

int main()
{
	std::string path = "/assets/geometry/H2S.cif";

	Cif2Geometry geom = Cif2Geometry(path);
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

	Atom *s = atoms->firstAtomWithName("S");
	Atom *hs1 = atoms->firstAtomWithName("HS1");
	Atom *hs2 = atoms->firstAtomWithName("HS2");
	
	glm::vec3 sp   = s->derivedPosition();
	glm::vec3 hs1p = hs1->derivedPosition();
	glm::vec3 hs2p = hs2->derivedPosition();
	
	float angle = glm::angle(hs1p - sp, hs2p - sp);

	BondAngle *bondAngle = s->bondAngle(0);
	
	float compare = bondAngle->angle();
	
	if (fabs(rad2deg(angle) - compare) > 1e-6)
	{
		std::cout << "Angles do not match: " << rad2deg(angle) << " vs "
		<< compare << std::endl;
		return 1;
	}
	
	delete atoms;
	
	return 0;
}

