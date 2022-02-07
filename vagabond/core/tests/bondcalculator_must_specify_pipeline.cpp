#include "../BondCalculator.h"
#include "../Atom.h"

int main()
{
	BondCalculator calculator;
	Atom a;
	
	try
	{
		calculator.setPipelineType(BondCalculator::PipelineNotSpecified);
		calculator.addAnchorExtension(&a, 0);
		calculator.setMaxSimultaneousThreads(1);
		calculator.setup();
	}
	catch (std::runtime_error err)
	{
		return 0;
	}
	
	return 1;
}
