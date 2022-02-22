#include "../Grid.h"

int main()
{
	try
	{
		Grid<double>(-5, 0, 0);
	}
	catch (std::exception err)
	{
		return 0;
	}

	return 1;
}
