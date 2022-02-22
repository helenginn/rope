#include "../RefList.h"
#include "../Diffraction.h"
#include <iostream>

int main()
{
	std::vector<Reflection> refs;
	refs.push_back(Reflection(-1, 2, 0));
	refs.push_back(Reflection(2, 4, 2));
	// max X = 2*2+1 = 5, Max Y = 2*4+1 = 9, Max Z = 2*2+1 = 5
	// therefore we would expect a grid of 6 x 10 x 6 for this reflection

	RefList list(refs);

	std::array<double, 6> cell = {16, 16, 16, 90, 90, 90};
	list.setUnitCell(cell);
	
	Diffraction diff(list);
	int nn = diff.nn();
	
	std::cout << "nn: " << nn << std::endl;
	
	return !(nn == 6 * 10 * 6);
}



