#include "../GeometryTable.h"

int main()
{
	GeometryTable table;
	table.addGeometryLength("COD", "A", "B", 2, 0.5);

	if (table.lengthExists("OTH", "A", "B") || 
	    !table.lengthExists("COD", "A", "B"))
	{
		return 1;
	}
	
	return 0;
}

