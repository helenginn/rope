#include "../CifFile.h"
#include "../AtomGroup.h"
#include "../BondCalculator.h"
#include <iostream>
#include <chrono>

int main()
{
	std::string path = "/assets/examples/4cvd.cif";

	CifFile geom = CifFile(path);
	geom.parse();
	
	AtomGroup *atoms = geom.atoms();
	Atom *anchor = atoms->possibleAnchor(0);

	std::chrono::high_resolution_clock::time_point start, prepared;
	start = std::chrono::high_resolution_clock::now();
	
	BondCalculator calculator;
	calculator.setPipelineType(BondCalculator::PipelineCalculatedMaps);
	calculator.setMaxSimultaneousThreads(2);
	calculator.setTotalSamples(120);
	calculator.addAnchorExtension(anchor);
	calculator.setup();
	
	calculator.start();
	
	Job job{};
	job.requests = JobCalculateMapSegment;
	const int num = 500;

	for (size_t i = 0; i < num; i++)
	{
		calculator.submitJob(job);
	}

	prepared = std::chrono::high_resolution_clock::now();
	
	Result *result = nullptr;
	
	for (size_t i = 0; i < num; i++)
	{
		result = calculator.acquireResult();
		if (result == nullptr)
		{
			std::cout << "Prematurely returned null result" << std::endl;
			return 1;
		}
//		std::cout << "Received ticket " << result->ticket << std::endl;

		result->destroy();
	}
	
//	std::cout << "Last ticket" << std::endl;
	
	result = calculator.acquireResult();
	
	if (result != nullptr)
	{
		std::cout << "Returned spurious extra result" << std::endl;
		return 1;
	}
	
	calculator.finish();

	std::chrono::high_resolution_clock::time_point end;
	end = std::chrono::high_resolution_clock::now();
	
	std::chrono::duration<double, std::milli> time_span = end - prepared;
	std::chrono::duration<double, std::milli> setup_span = prepared - start;
	
	std::cout << "Setup time (ms): " <<  setup_span.count() << std::endl;
	std::cout << "Run time (ms): " <<  time_span.count() << std::endl;

	return 0;
}
