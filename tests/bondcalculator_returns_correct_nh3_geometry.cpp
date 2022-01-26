#include "../src/Cif2Geometry.h"
#include "../src/AtomGroup.h"
#include "../src/BondAngle.h"
#include "../src/BondCalculator.h"

int main()
{
	std::string path = "/assets/geometry/NH3.cif";

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

	Atom *n = atoms->firstAtomWithName("N");
	Atom *hn1 = atoms->firstAtomWithName("HN1");
	Atom *hn2 = atoms->firstAtomWithName("HN2");
	Atom *hn3 = atoms->firstAtomWithName("HN3");
	
	glm::vec3 np   = n->derivedPosition();
	glm::vec3 hn1p = hn1->derivedPosition();
	glm::vec3 hn2p = hn2->derivedPosition();
	glm::vec3 hn3p = hn3->derivedPosition();
	
	float angle[3];
	angle[0] = glm::angle(hn1p - np, hn2p - np);
	angle[1] = glm::angle(hn2p - np, hn3p - np);
	angle[2] = glm::angle(hn3p - np, hn1p - np);

	BondAngle *bondAngle12 = n->findBondAngle(hn1, n, hn2);
	BondAngle *bondAngle23 = n->findBondAngle(hn2, n, hn3);
	BondAngle *bondAngle31 = n->findBondAngle(hn3, n, hn1);
	
	float compare[3];
	compare[0] = bondAngle12->angle();
	compare[1] = bondAngle23->angle();
	compare[2] = bondAngle31->angle();
	
	for (size_t i = 0; i < 3; i++)
	{
		if (fabs(rad2deg(angle[i]) - compare[i]) > 1e-6)
		{
			std::cout << "Angles do not match: " << rad2deg(angle[i]) << " vs "
			<< compare[i] << std::endl;
			return 1;
		}
	}

	delete atoms;
	
	return 0;
}

