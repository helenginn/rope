#include "../File.h"
#include "../GeometryTable.h"

int main()
{
	std::string path = "/assets/examples/4cvd.pdb";

	File *f = File::loadUnknown(path);
	
	if (f == nullptr)
	{
		std::cout << "File f is NULL" << std::endl;
		return 1;
	}
	
	size_t n = f->atomCount();
	return !(n > 0);
}

