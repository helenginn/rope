#include "../DataGroup.h"
#include <iostream>

int main()
{
	size_t length = 3;
	size_t num = 4;
	DataGroup<float> dg(length);
	
	dg.addArray("zero", {-4, 0, 4});
	dg.addArray("one", {2, 0, -2});
	dg.addArray("two", {1, 0, -1});
	dg.addArray("three", {-2, 0, 2});
	
	dg.normalise();
	
	PCA::Matrix m = dg.correlationMatrix();

	double expected[] = {0, -1, -1, 1, -1, 0, 1, -1, -1, 1, 0, -1, 1, -1, -1, 0};
	
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


