#include "../Cif2Geometry.h"
#include "../GeometryTable.h"

int main()
{
	std::string path = "/assets/geometry/ASP.cif";

	Cif2Geometry geom = Cif2Geometry(path);
	geom.parse();
	
	GeometryTable *table = geom.geometryTable();
	double torsion = table->torsion("ASP", "N", "CA", "CB", "CG");
	
	return !(fabs(torsion - 180) < 1e-6);
}
