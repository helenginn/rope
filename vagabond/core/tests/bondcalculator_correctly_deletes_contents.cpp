#include "../BondSequenceHandler.h"
#include "../CifFile.h"
#include "../AtomGroup.h"
#include "../BondSequence.h"
#include "../BondCalculator.h"
#include <iostream>

int main()
{
	std::string path = "/assets/geometry/GLY.cif";

	CifFile geom = CifFile(path);
	geom.parse();
	
	AtomGroup *atoms = geom.atoms();
	Atom *anchor = atoms->firstAtomWithName("OXT");

	for (size_t i = 0; i < 10; i++)
	{
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

		calculator.finish();
	}

	return 0;
}
