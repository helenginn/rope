#include "../ElementLibrary.h"
#include <iostream>

int main()
{
	ElementLibrary &library = ElementLibrary::library();

	const float *factors = library.getElementFactors("N");
	
	if (fabs(factors[0] - 7) > 1e-6)
	{
		std::cout << "Nitrogen scattering factor fail" << std::endl;
		std::cout << factors[0] << std::endl;
		return 1;
	}
	
	const float &last = factors[library.numPoints() - 1];
	
	if (fabs(last - 0.023) > 1e-6)
	{
		std::cout << "Nitrogen last scattering factor fail" << std::endl;
		std::cout << last << std::endl;
		return 1;
	}
}
