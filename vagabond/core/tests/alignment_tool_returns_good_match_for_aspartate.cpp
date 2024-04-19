#include "../AtomGroup.h"
#include "../files/CifFile.h"
#include "../BondCalculator.h"
#include "../AlignmentTool.h"
#include <iomanip>

int main()
{
	std::string path = "/assets/geometry/ASP.cif";

	CifFile geom = CifFile(path);
	geom.parse();
	
	AtomGroup &aspartate = *geom.atoms();

	BondCalculator calculator;
	calculator.setPipelineType(BondCalculator::PipelineAtomPositions);
	calculator.setMaxSimultaneousThreads(4);
	calculator.setTotalSamples(1);
	calculator.addAnchorExtension(aspartate.possibleAnchor(0));
	calculator.setup();
	
	calculator.start();
	
	Job job{};
	job.requests = JobCalculateDeviations;
	int tickets[2];
	tickets[0] = calculator.submitJob(job);

	AlignmentTool tool(&aspartate);
	tool.run();
	tickets[1] = calculator.submitJob(job);
	
	Result *result = nullptr;
	double devs[2];

	while ((result = calculator.acquireObject()))
	{
		for (size_t i = 0; i < 2; i++)
		{
			if (result->ticket == tickets[i])
			{
				devs[i] = result->deviation;
			}
		}
	}
	
	std::cout << "Deviation from " << devs[0] << " to " << devs[1] << 
	" Angstroms after alignment." << std::endl;
	
	return !(devs[1] < devs[0]);
}
