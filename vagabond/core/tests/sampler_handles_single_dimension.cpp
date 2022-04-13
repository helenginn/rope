#include <iostream>
#include <cmath>
#include <vagabond/core/Sampler.h>

int main()
{
	float manual = M_PI;
	int dims = 1;
	
	Sampler sampler(120, dims);
	sampler.setup();
	
	int n = sampler.pointCount();
	std::cout << "Sample count: " << n << std::endl;
	
	return 0;
}
