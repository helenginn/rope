#include "../src/Cif2Geometry.h"
#include "../src/AtomGroup.h"
#include "../src/BondCalculator.h"
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
	calculator.setMaxSimultaneousThreads(5);
	calculator.addAnchorExtension(anchor);
	calculator.setup();
	
	calculator.start();
	
	Job empty_job{};
	const int num = 1000;

	for (size_t i = 0; i < num; i++)
	{
		calculator.submitJob(empty_job);
	}
	
	Result *result = nullptr;
	
	for (size_t i = 0; i < num; i++)
	{
		result = calculator.acquireResult();
		if (result == nullptr)
		{
			std::cout << "Prematurely returned null result" << std::endl;
			return 1;
		}
		std::cout << "Received ticket " << result->ticket << std::endl;

		delete result;
	}
	
	result = calculator.acquireResult();
	
	if (result != nullptr)
	{
		std::cout << "Returned spurious extra result" << std::endl;
		return 1;
	}
	
	calculator.finish();

	return 0;
}
