#include "../Grid.h"

int main()
{
	Grid<double> grid = Grid<double>(2, 2, 2);
	
	bool within = grid.withinBounds(-1, -1, -1);

	return !(within);
}


