#include "../GeometryTable.h"

int main()
{
	GeometryTable table;
	table.addGeometryAngle("COD", "A", "B", "C", 2, 0.5);

	double val = table.angle("COD", "A", "B", "C");
	double diff = (val - 2);

	return !(fabs(diff) < 1e-6);
}



