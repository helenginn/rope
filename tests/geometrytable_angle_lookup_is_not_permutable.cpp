#include "../src/GeometryTable.h"

int main()
{
	GeometryTable table;
	table.addGeometryAngle("COD", "A", "B", "C", 2, 0.5);

	if (table.angleExists("COD", "B", "C", "A") ||
	    !table.angleExists("COD", "A", "B", "C"))
	{
		return 1;
	}

	return 0;
}


