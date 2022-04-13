#include <iostream>
#include <cmath>
#include <vagabond/core/Sampler.h>

int main()
{
	int want = 120;
	Sampler sampler(want, 3);
	sampler.setup();

	int samples = sampler.pointCount();
	std::cout << "Samples: " << samples << std::endl;
	int ratio = (float)samples / (float)want;
	
	return !(ratio < 1.05 && ratio > 0.95);
}
