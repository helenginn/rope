#include "../RefList.h"
#include <iostream>

int main()
{
	std::vector<Reflection> refs;
	refs.push_back(Reflection(6, 4, 12));

	RefList list(refs);

	std::array<double, 6> cell = {16, 16, 26, 90, 90, 120};
	list.setSpaceGroup(168); // P 6
	list.setUnitCell(cell);
	
	double res = list.resolutionOf(0);
	glm::vec3 frac = list.reflAsFraction(0);
	
	for (size_t i = 0; i < list.symOpCount(); i++)
	{
		glm::vec3 rot = list.applyRotSym(frac, i);
		double next = list.resolutionOf(rot);

		if (fabs(res - next) > 1e-2)
		{
			std::cout << "Resolution of symop is not identical. " 
			<< res << " vs " << next << std::endl;
			return 1;
		}
		else
		{
			std::cout << "Match for symop " << i << std::endl;
		}
	}
	
	return 0;
}


