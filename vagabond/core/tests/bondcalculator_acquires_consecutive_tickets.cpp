#include "../BondCalculator.h"

int main()
{
	BondCalculator calculator;
	calculator.setPipelineType(BondCalculator::PipelineAtomPositions);
	calculator.setMaxSimultaneousThreads(1);
	calculator.setup();
	calculator.start();

	Job empty_job{};

	calculator.submitJob(empty_job);
	calculator.submitJob(empty_job);
	
	for (size_t i = 0; i < 2; i++)
	{
		Job *job = calculator.acquireJob();

		if (job->ticket != i)
		{
			std::cout << "Job ticket: " << job->ticket << " (should be " << i 
			<< ")" << std::endl;
			return 1;
		}
	}
	
	return 0;
}

