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
	calculator.setPipelineType(BondCalculator::PipelineAtomPositions);
	calculator.setMaxSimultaneousThreads(4);
	calculator.setTotalSamples(2);
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
	
	std::cout << "Last ticket" << std::endl;
	
	result = calculator.acquireResult();
	
	if (result != nullptr)
	{
		std::cout << "Returned spurious extra result" << std::endl;
		return 1;
	}
	
	calculator.finish();

	std::chrono::high_resolution_clock::time_point end;
	end = std::chrono::high_resolution_clock::now();
	
	std::chrono::duration<double, std::milli> time_span = end - start;
	
	std::cout << "Milliseconds taken: " <<  time_span.count() << std::endl;


	return 0;
}
