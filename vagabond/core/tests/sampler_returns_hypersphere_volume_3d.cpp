#include <vagabond/core/Sampler.h>
#include <cmath>
#include <iostream>

int main()
{
	float manual = 4. / 3. * M_PI;
	
	Sampler sampler(1, 3);
	float calc = sampler.hypersphereVolume(1);

	float ratio = calc / manual;
	
	std::cout << "Manual 3D volume: " << manual << std::endl;
	std::cout << "Calculated volume: " << calc << std::endl;
	std::cout << "Ratio: " << ratio << std::endl;
	
	return !(ratio < 1.1 && ratio > 1);
}
