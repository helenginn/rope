#include "../Knotter.h"
#include "../Cif2Geometry.h"
#include "../AtomGroup.h"
#include "../GeometryTable.h"

int main()
{
	std::string path = "/assets/geometry/ASP.cif";

	Cif2Geometry geom = Cif2Geometry(path);
	geom.parse();
	
	Knotter knotter(geom.atoms(), geom.geometryTable());
	knotter.knot();

	int warns = knotter.warningCount();
	
	delete geom.atoms();
	delete geom.geometryTable();
	
	return !(warns == 0);
}
