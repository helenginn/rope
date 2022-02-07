#include "../GeometryTable.h"

int main()
{
	GeometryTable table;
	table.addGeometryAngle("COD", "A", "B", "C", 2, 0.5);

	if (!table.angleExists("COD", "C", "B", "A"))
	{
		return 1;
	}

	float diff = table.angle("COD", "C", "B", "A") - 2;
	if (fabs(diff) > 1e-6)
	{
		return 1;
	}
	
	return 0;
}

