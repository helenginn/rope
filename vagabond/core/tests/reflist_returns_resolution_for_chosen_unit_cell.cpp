#include "../RefList.h"
#include <iostream>

int main()
{
	std::vector<Reflection> refs;
	refs.push_back(Reflection(2, 2, 2));

	RefList list(refs);

	std::array<double, 6> cell = {16, 16, 16, 90, 90, 90};
	list.setUnitCell(cell);
	
	double res = list.resolutionOf(0);
	double target = 4.6188;
	
	if (fabs(res - target) > 1e-2)
	{
		std::cout << "Received res " << res << std::endl;
		return 1;
	}
	
	return 0;
}


