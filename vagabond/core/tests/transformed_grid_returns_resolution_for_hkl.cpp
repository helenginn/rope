#include "../TransformedGrid.h"
#include "../matrix_functions.h"
#include <iostream>

int main()
{
	TransformedGrid<double> grid = TransformedGrid<double>(2, 2, 2);
	
	glm::mat3x3 cell = mat3x3_from_unit_cell(100, 100, 100, 90, 90, 90);
	grid.setRealMatrix(cell);
	
	double res = grid.resolution(10, 0, 0);
	
	std::cout << res << " Ang " << std::endl;

	return !(fabs(res - 10) < 1e-2);
}


