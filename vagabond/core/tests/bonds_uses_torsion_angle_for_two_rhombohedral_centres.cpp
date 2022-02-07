#include <iostream>
#include "../matrix_functions.h"
#include "../BondTorsion.h"
#include "../Knotter.h"
#include "../AtomGroup.h"
#include "../GeometryTable.h"
#include "../Atom.h"

int main()
{
	Atom a("TST", "A"), a1("TST", "A1"), a2("TST", "A2"), a3("TST", "A3");
	Atom b("TST", "B"), b1("TST", "B1"), b2("TST", "B2"), b3("TST", "B3");
	a1.setElementSymbol("S");
	a2.setElementSymbol("H");
	a3.setElementSymbol("H");
	b2.setElementSymbol("H");
	b3.setElementSymbol("H");
	b1.setElementSymbol("S");
	b.setElementSymbol("C");
	a.setElementSymbol("C");
	
	AtomGroup group;
	group += &a; group += &a1; group += &a2; group += &a3;
	group += &b; group += &b1; group += &b2; group += &b3;

	GeometryTable table;
	table.addGeometryLength("TST", "A", "A1", 1.5, 0);
	table.addGeometryLength("TST", "A", "A2", 1.5, 0);
	table.addGeometryLength("TST", "A", "A3", 1.5, 0);

	table.addGeometryLength("TST", "B", "B1", 1.5, 0);
	table.addGeometryLength("TST", "B", "B2", 1.5, 0);
	table.addGeometryLength("TST", "B", "B3", 1.5, 0);
	
	table.addGeometryLength("TST", "B", "A", 1.5, 0);

	table.addGeometryAngle("TST", "A1", "A", "B", 109.5, 0);
	table.addGeometryAngle("TST", "A2", "A", "B", 109.5, 0);
	table.addGeometryAngle("TST", "A3", "A", "B", 109.5, 0);
	table.addGeometryAngle("TST", "A1", "A", "A2", 109.5, 0);
	table.addGeometryAngle("TST", "A2", "A", "A3", 109.5, 0);
	table.addGeometryAngle("TST", "A3", "A", "A1", 109.5, 0);

	table.addGeometryAngle("TST", "B1", "B", "A", 109.5, 0);
	table.addGeometryAngle("TST", "B2", "B", "A", 109.5, 0);
	table.addGeometryAngle("TST", "B3", "B", "A", 109.5, 0);
	table.addGeometryAngle("TST", "B1", "B", "B2", 109.5, 0);
	table.addGeometryAngle("TST", "B2", "B", "B3", 109.5, 0);
	table.addGeometryAngle("TST", "B3", "B", "B1", 109.5, 0);
	
	table.addGeometryTorsion("TST", "B1", "B", "A", "A1", 35, 0, 1);
	
	Knotter knotter(&group, &table);
	knotter.knot();
	
	group.recalculate();

	bool bad = false;
	for (size_t i = 0; i < group.size(); i++)
	{
		Atom *a = group[i];
		glm::vec3 d = a->derivedPosition();
		std::cout << a->atomName() << " " << glm::to_string(d) << std::endl;
		
		if (!is_glm_vec_sane(d))
		{
			bad = true;
		}
	}

	BondTorsion torsion(nullptr, &a1, &a, &b, &b1, 0);
	double measured = torsion.measurement(BondTorsion::SourceDerived);
	double angle = 35;
	
	if (fabs(angle - measured) > 1e-2)
	{
		std::cout << "Measured: " << measured << std::endl;
		std::cout << "Actual: " << angle << std::endl;
		bad = true;
	}
	
	return bad;
}
