#include "../DataGroup.h"
#include <iostream>

int main()
{
	size_t num = 4;
	DataGroup<float> dg(num);
	
	dg.addArray("zero", {0, 0, 0, 0});
	dg.addArray("one", {1, 1, 1, 1});
	dg.addArray("two", {2, 2, 2, 2});
	
	DataGroup<float>::Array ave = dg.average();
	
	if (ave.size() != num)
	{
		std::cout << "Average size is incorrect" << std::endl;
		return 1;
	}
	
	for (size_t i = 0; i < num; i++)
	{
		if (fabs(ave[i] - 1) > 1e-6)
		{
			std::cout << "Average number not 1: " << ave[i] << std::endl;
			return 1;
		}
	}

	return 0;
}
