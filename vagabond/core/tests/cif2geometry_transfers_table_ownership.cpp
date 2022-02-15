#include "../CifFile.h"
#include "../GeometryTable.h"

int main()
{
	CifFile *geom = new CifFile("");
	GeometryTable *table = geom->geometryTable();
	delete geom;
	
	bool exists = table->lengthExists("", "", "");
	
	return !(exists == false);
}

