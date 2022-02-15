#include "../CifFile.h"
#include "../AtomGroup.h"
#include "../BondSequence.h"
#include "../BondSequenceHandler.h"
#include "../BondCalculator.h"

int main()
{
	std::string path = "/assets/geometry/GLY.cif";

	CifFile geom = CifFile(path);
	geom.setAutomaticKnot(true);
	geom.parse();
	
	AtomGroup *atoms = geom.atoms();
	Atom *anchor = atoms->firstAtomWithName("OXT");
	
	BondCalculator calculator;
	calculator.setPipelineType(BondCalculator::PipelineAtomPositions);
	calculator.setMaxSimultaneousThreads(1);
	calculator.addAnchorExtension(anchor);
	calculator.setup();

	const BondSequence *sequence = calculator.sequenceHandler()->sequence(0);
	int added = sequence->addedAtomsCount();
	
	delete atoms;
	
	return !(added == 10);
}

