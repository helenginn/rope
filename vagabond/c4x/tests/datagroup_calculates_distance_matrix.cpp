#include "../DataGroup.h"
#include <iostream>

int main()
{
	size_t length = 3;
	size_t num = 3;
	DataGroup<float> dg(length);
	
	dg.addArray("zero", {0, 0, 0});
	dg.addArray("one", {0, 3, 0});
	dg.addArray("two", {0, 0, 4});
	
	PCA::Matrix m = dg.distanceMatrix();

	double expected[] = {0, 3, 4, 3, 0, 5, 4, 5, 0};
	
	for (size_t i = 0; i < num; i++)
	{
		for (size_t j = 0; j < num; j++)
		{
			double target = expected[i * num + j];
			double have = m[i][j];

			if (fabs(have - target) > 1e-6)
			{
				std::cout << "Have " << have << " but target is " << 
				target << std::endl;
				return 1;
			}
		}
	}

	return 0;
}


