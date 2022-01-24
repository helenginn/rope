#include "../src/BondCalculator.h"
#include "../src/Atom.h"

int main()
{
	BondCalculator calculator;
	Atom a;
	
	try
	{
		calculator.setPipelineType(BondCalculator::PipelineAtomPositions);
		calculator.addAnchorExtension(&a, 0);
		calculator.setMaxSimultaneousThreads(0);
		calculator.setup();
	}
	catch (std::runtime_error err)
	{
		return 0;
	}
	
	return 1;
}
