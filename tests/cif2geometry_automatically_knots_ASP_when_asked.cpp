#include "../src/Cif2Geometry.h"
#include "../src/AtomGroup.h"

int main()
{
	std::string path = "/assets/geometry/ASP.cif";

	Cif2Geometry geom = Cif2Geometry(path);
	geom.setAutomaticKnot(true);
	geom.parse();
	
	AtomGroup *atoms = geom.atoms();
	
	return !(atoms->bondstraintCount() > 0);
}
