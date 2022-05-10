#include "../DataGroup.h"
#include <iostream>

int main()
{
	size_t num = 4;
	DataGroup<float> dg(num);
	
	dg.addArray("zero", {0, -1, -2, -3});
	dg.addArray("one", {1, 1, 1, 1});
	dg.addArray("two", {2, 3, 4, 5});
	
	dg.normalise();
	
	DataGroup<float>::Array arr = dg.differenceVector(0);
	
	for (size_t i = 0; i < num; i++)
	{
		float target = arr[0];
		
		if (fabs(arr[i] - target) > 1e-6)
		{
			std::cout << "All values of array should be identical, but we have "
			<< arr[i] << " not " << arr[0] << std::endl;
			return 1;
		}
	}

	return 0;
}

