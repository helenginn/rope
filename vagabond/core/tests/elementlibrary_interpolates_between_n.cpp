#include "../ElementLibrary.h"
#include <iostream>

int main()
{
	ElementLibrary &library = ElementLibrary::library();

	const float *factors = library.getElementFactors("N");
	
	float val = library.valueForResolution(0.005, factors);
	
	if (fabs(val - 6.9955) > 1e-6)
	{
		std::cout << "Nitrogen last scattering factor fail" << std::endl;
		std::cout << val << std::endl;
		return 1;
	}
}
