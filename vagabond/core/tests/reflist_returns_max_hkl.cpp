#include "../RefList.h"
#include <iostream>

int main()
{
	std::vector<Reflection> refs;
	
	refs.push_back(Reflection(0, 3, 5));
	refs.push_back(Reflection(-5, 0, 3));
	refs.push_back(Reflection(-3, 5, 0));

	RefList list(refs);
	
	HKL max = list.maxHKL();
	
	if (max.h != 5 || max.k != 5 || max.l != 5)
	{
		std::cout << max.h << " " << max.k << " " << max.l << std::endl;
		return 1;
	}
	
	return 0;
}

