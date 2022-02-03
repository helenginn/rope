#include "../src/Cif2Geometry.h"
#include "../src/AtomGroup.h"
#include "../src/BondSequence.h"
#include "../src/BondSequenceHandler.h"
#include "../src/BondCalculator.h"

int main()
{
	std::string path = "/assets/geometry/GLY.cif";

	Cif2Geometry geom = Cif2Geometry(path);
	geom.parse();
	
	AtomGroup *atoms = geom.atoms();
	Atom *anchor = atoms->firstAtomWithName("N");
	
	BondCalculator calculator;
	calculator.setPipelineType(BondCalculator::PipelineAtomPositions);
	calculator.setMaxSimultaneousThreads(1);
	calculator.addAnchorExtension(anchor);
	calculator.setup();

	calculator.setMinMaxDepth(0, 4);
	calculator.start();

	std::vector<bool> mask = calculator.sequenceHandler()->depthLimitMask();
	
	if (mask.size() == 0)
	{
		std::cout << "Returned mask was 0 size" << std::endl;
		return 1;
	}
	
	int found_zero = 0;
	int found_one = 0;
	
	for (bool m : mask)
	{
		found_zero += (m ? 0 : 1);
		found_one += (m ? 1 : 0);
	}
	
	if (found_zero == 0 || found_one == 0)
	{
		std::cout << "Bad mask: " << found_zero << " zeros and " << 
		found_one << " ones." << std::endl;
		return 1;
	}
	
	return 0;
}

