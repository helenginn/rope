#include "../src/GeometryTable.h"

int main()
{
	GeometryTable table;
	table.addGeometryTorsion("COD", "A", "B", "C", "D", 2, 0.5, 1);

	double val = table.torsion("COD", "A", "B", "C", "D");
	double diff = (val - 2);

	return !(fabs(diff) < 1e-6);
}




