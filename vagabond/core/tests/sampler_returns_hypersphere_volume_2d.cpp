#include <iostream>
#include <cmath>
#include <vagabond/core/Sampler.h>

int main()
{
	float manual = M_PI;
	
	Sampler sampler(1, 2);
	float calc = sampler.hypersphereVolume(1);

	float ratio = calc / manual;
	
	std::cout << "Manual volume: " << manual << std::endl;
	std::cout << "Calculated volume: " << calc << std::endl;
	std::cout << "Ratio: " << ratio << std::endl;
	
	return !(ratio < 1.1 && ratio > 1);
}
