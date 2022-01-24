#include "../src/GeometryTable.h"

int main()
{
	GeometryTable table;
	table.addGeometryLength("COD", "A", "B", 2, 0.5);

	if (!table.lengthExists("COD", "B", "A"))
	{
		return 1;
	}

	float diff = table.length("COD", "B", "A") - 2;
	if (fabs(diff) > 1e-6)
	{
		return 1;
	}
	
	return 0;
}

