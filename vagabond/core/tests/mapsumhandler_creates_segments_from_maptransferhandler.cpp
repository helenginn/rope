#include "../CifFile.h"
#include "../AtomGroup.h"
#include "../BondCalculator.h"
#include <iostream>
#include <chrono>

int main()
{
	std::string path = "/assets/geometry/LSD.cif";

	CifFile geom = CifFile(path);
	geom.setAutomaticKnot(true);
	geom.parse();
	
	AtomGroup *atoms = geom.atoms();
	Atom *anchor = atoms->possibleAnchor(0);

	std::chrono::high_resolution_clock::time_point start;
	start = std::chrono::high_resolution_clock::now();
	
	BondCalculator calculator;
	calculator.setPipelineType(BondCalculator::PipelineCalculatedMaps);
	calculator.setMaxSimultaneousThreads(1);
	calculator.setTotalSamples(1);
	calculator.addAnchorExtension(anchor);
	calculator.setup();
	
	calculator.start();
	
	MapSumHandler *msh = calculator.sumHandler();

	return 0;
}

