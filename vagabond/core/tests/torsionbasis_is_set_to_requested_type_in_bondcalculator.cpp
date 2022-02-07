#include "../Cif2Geometry.h"
#include "../AtomGroup.h"
#include "../BondSequence.h"
#include "../BondSequenceHandler.h"
#include "../ConcertedBasis.h"
#include "../BondCalculator.h"

int main()
{
	std::string path = "/assets/geometry/GLY.cif";

	Cif2Geometry geom = Cif2Geometry(path);
	geom.parse();
	
	AtomGroup *atoms = geom.atoms();
	Atom *anchor = atoms->firstAtomWithName("OXT");
	
	BondCalculator calculator;
	calculator.setPipelineType(BondCalculator::PipelineAtomPositions);
	calculator.setMaxSimultaneousThreads(1);
	calculator.addAnchorExtension(anchor);
	calculator.setTorsionBasisType(TorsionBasis::TypeConcerted);
	calculator.setup();

	const BondSequence *sequence = calculator.sequenceHandler()->sequence(0);
	const TorsionBasis *basis = sequence->torsionBasis();
	delete atoms;
	
	if (dynamic_cast<const ConcertedBasis *>(basis) != nullptr)
	{
		return 0;
	}
	
	return 1;
}

