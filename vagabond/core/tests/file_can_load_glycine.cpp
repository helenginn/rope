#include "../File.h"
#include "../GeometryTable.h"

int main()
{
	std::string path = "/assets/geometry/GLY.cif";

	File *f = File::loadUnknown(path);
	
	if (f == nullptr)
	{
		std::cout << "File f is NULL" << std::endl;
		return 1;
	}
	
	GeometryTable *table = f->geometryTable();
	return !(table->codeEntries() > 0);
}

