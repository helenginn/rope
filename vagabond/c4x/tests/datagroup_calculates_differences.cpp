#include "../DataGroup.h"
#include <iostream>

int main()
{
	size_t num = 4;
	DataGroup<float> dg(num);
	
	dg.addArray("zero", {0, 1, 0, 1});
	dg.addArray("one", {1, 1, 1, 1});
	dg.addArray("two", {2, 1, 2, 1});
	
	dg.findDifferences();
	
	DataGroup<float>::Array arr = dg.differenceVector(0);
	
	for (size_t i = 0; i < num; i++)
	{
		float target = (i % 2 == 0 ? -1 : 0);
		
		if (fabs(arr[i] - target) > 1e-6)
		{
			std::cout << "Wrong value for array[" << i << "]: " <<
			arr[i] << " not " << target << std::endl;
			return 1;
		}
	}

	return 0;
}
