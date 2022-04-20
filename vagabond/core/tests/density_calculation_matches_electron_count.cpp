#include <vagabond/core/Sampler.h>
#include <vagabond/core/AtomGroup.h>
#include <vagabond/core/BondCalculator.h>
#include <vagabond/core/AtomMap.h>
#include <vagabond/core/CifFile.h>

int main()
{
	std::string path = "/assets/geometry/GLY.cif";
	CifFile geom = CifFile(path);
	geom.parse();

	AtomGroup *atoms = geom.atoms();
	Atom *anchor = atoms->chosenAnchor();

	BondCalculator calculator;
	calculator.setPipelineType(BondCalculator::PipelineCalculatedMaps);
	calculator.setMaxSimultaneousThreads(1);
	calculator.setTorsionBasisType(TorsionBasis::TypeSimple);
	calculator.setTotalSamples(1);
	calculator.addAnchorExtension(anchor);
	calculator.setup();
	calculator.start();

	Job job{};
	job.requests = static_cast<JobType>(JobCalculateMapSegment);
	calculator.submitJob(job);

	Result *result = calculator.acquireResult();
	AtomMap *map = result->map;
	double mass = map->sum();
	
	result->destroy();
	calculator.finish();

	float target = 40;
	std::cout << "Target mass: " << target << " Da." << std::endl;
	std::cout << "Actual mass: " << mass << " Da." << std::endl;

	if (fabs(target - mass) > 1e-3)
	{
		
		return 1;
	}

	return 0;
}

