#include "../src/Cif2Geometry.h"
#include "../src/AtomGroup.h"
#include "../src/Atom.h"
#include "../src/BondCalculator.h"
#include "../src/matrix_functions.h"
#include <iostream>

int main()
{
	std::string path = "/assets/geometry/GLY.cif";

	Cif2Geometry geom = Cif2Geometry(path);
	geom.setAutomaticKnot(true);
	geom.parse();
	
	AtomGroup *atoms = geom.atoms();
	Atom *anchor = atoms->firstAtomWithName("OXT");

	BondCalculator calculator;
	calculator.setPipelineType(BondCalculator::PipelineAtomPositions);
	calculator.setMaxSimultaneousThreads(1);
	calculator.addAnchorExtension(anchor);
	calculator.setup();

	calculator.start();

	Job empty_job{};
	calculator.submitJob(empty_job);
	
	Result *result = calculator.acquireResult();
	std::cout << "Ticket: " << result->ticket << std::endl;
	std::cout << "Atom count: " << result->aps.size() << std::endl;
	
	for (size_t i = 0; i < result->aps.size(); i++)
	{
		Atom *a = result->aps[i].atom;
		glm::vec3 v = result->aps[i].pos;
		
		std::cout << a->atomName() << " " << glm::to_string(v) << std::endl;
		
		if (!is_glm_vec_sane(v))
		{
			return 1;
		}
	}

	calculator.finish();

	return 0;
}
