#include "../src/GeometryTable.h"

int main()
{
	GeometryTable table;
	table.addGeometryTorsion("COD", "A", "B", "C", "D", 2, 0.5, 1);

	if (!table.torsionExists("COD", "D", "C", "B", "A"))
	{
		return 1;
	}

	float diff = table.torsion("COD", "D", "C", "B", "A") - 2;
	if (fabs(diff) > 1e-6)
	{
		return 1;
	}
	
	return 0;

}
