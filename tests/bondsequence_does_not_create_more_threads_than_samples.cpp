#include "../src/BondCalculator.h"
#include "../src/BondSequenceHandler.h"
#include "../src/Atom.h"

int main()
{
	BondCalculator calculator;
	Atom a;
	
	calculator.setPipelineType(BondCalculator::PipelineAtomPositions);
	calculator.addAnchorExtension(&a, 0);
	calculator.setMaxSimultaneousThreads(5);
	calculator.setTotalSamples(1);
	calculator.setup();
	
	BondSequenceHandler *handler = calculator.sequenceHandler();

	int threadCount = handler->threadCount();
	
	if (threadCount != 1)
	{
		return 1;
	}
	
	return 0;
}
