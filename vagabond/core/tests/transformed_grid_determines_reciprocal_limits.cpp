#include "../TransformedGrid.h"
#include <iostream>

int main()
{
	TransformedGrid<double> grid = TransformedGrid<double>(20, 20, 20);
	
	int limit = grid.reciprocalLimitIndex(0);
	
	return !(limit == 10);
}



