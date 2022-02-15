#include "../AtomGroup.h"
#include "../CifFile.h"
#include "../AlignmentTool.h"
#include "../PositionRefinery.h"
#include "../TorsionBasis.h"
#include <thread>

int main()
{
	std::string path = "/assets/geometry/ATP.cif";

	CifFile geom = CifFile(path);
	geom.parse();
	
	AtomGroup &atoms = *geom.atoms();
	AlignmentTool tool(&atoms);
	tool.run();

	atoms.recalculate();
	PositionRefinery *refinery = new PositionRefinery(&atoms);
	refinery->setTorsionBasis(TorsionBasis::TypeConcerted);
	std::thread *refine = new std::thread(&PositionRefinery::backgroundRefine, 
	                                      refinery);
	
	refine->join();
}

