#include "../ElementSegment.h"
#include "../FFT.h"
#include "../AtomGroup.h"
#include "../Atom.h"

int main()
{
	AtomGroup test;
	Atom a;
	test += &a;
	
	ElementSegment segment;
	segment.setStatus(FFT<VoxelElement>::Real);

	segment.setDimensions(2, 2, 2);
	segment.makePlans();

	double density = 4;
	segment.addDensity(glm::vec3(0.5, 0.5, 0.5), density);
//	segment.addDensity(glm::vec3(-0.5, -0.5, -0.5), density);

	float sum = segment.sum();
	if (fabs(sum - 4) > 1e-5)
	{
		std::cout << "Sum (should be 4): " << sum << std::endl;
		return 1;
	}
	
	for (size_t i = 0; i < segment.nn(); i++)
	{
		VoxelElement &ve = segment.element(i);
		
		if (fabs(ve.value[0] - 0.5) > 1e-6)
		{
			std::cout << "value should be 0.5, " << std::endl;
			std::cout << "... actually " << ve.value[0] << std::endl;

			return 1;
		}
	}
	
	std::cout << "Good so far" << std::endl;
	
	segment.fft();
	segment.fft();
	
	float compare = segment.sum();
	std::cout << "Sum after 2x FFT (real): " << compare << std::endl;

	return !(fabs(compare - sum) < 1e-5);
}


