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

	double density = 5;
	segment.addDensity(glm::vec3(0., 0., 0.), density);

	float sum = segment.sum();
	std::cout << "Sum (real): " << sum << std::endl;
	
	segment.fft();
	segment.fft();
	
	float compare = segment.sum();
	std::cout << "Sum after 2x FFT (real): " << compare << std::endl;

	return !(fabs(compare - sum) < 1e-5);
}

