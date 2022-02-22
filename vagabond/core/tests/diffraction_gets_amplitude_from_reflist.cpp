#include "../RefList.h"
#include "../Diffraction.h"
#include <iostream>

int main()
{
	std::vector<Reflection> refs;
	refs.push_back(Reflection(-1, 2, 0));
	refs.push_back(Reflection(2, 4, 2, 100));
	// max X = 2*2+1 = 5, Max Y = 2*4+1 = 9, Max Z = 2*2+1 = 5
	// therefore we would expect a grid of 6 x 10 x 6 for this reflection

	RefList list(refs);
	Diffraction diff(list);
	
	VoxelDiffraction &v = diff.element(2, 4, 2);
	float amp = v.amplitude();
	
	std::cout << "Amplitude: " << amp << std::endl;
	
	return !(fabs(amp - 100) < 1e-6);
}



