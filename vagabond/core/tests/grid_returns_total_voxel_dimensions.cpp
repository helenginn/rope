#include "../Grid.h"

int main()
{
	Grid<double> grid = Grid<double>(2, 2, 2);
	
	int total = grid.nn();

	return !(total == 8);
}

