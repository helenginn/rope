#include <iostream>
#include "../matrix_functions.h"
#include "../BondTorsion.h"
#include "../Grapher.h"
#include "../Knotter.h"
#include "../AtomGroup.h"
#include "../GeometryTable.h"
#include "../Atom.h"

bool measurement_matches(BondTorsion torsion, double angle)
{
	double measured = torsion.measurement(BondTorsion::SourceDerived);
	bool bad = false;
	
	if (fabs(angle - measured) > 1e-3)
	{
		std::cout << "Failure on angle " << torsion.desc() << std::endl;
		std::cout << "Measured: " << measured << std::endl;
		std::cout << "Actual: " << angle << std::endl;
		bad = true;
	}

	return bad;
}

int main()
{
	Atom a("TST", "A"), a1("TST", "A1"), a2("TST", "A2"), a3("TST", "A3");
	Atom b("TST", "B"), b1("TST", "B1"), b2("TST", "B2");
	Atom c("TST", "C"), c1("TST", "C1"), c2("TST", "C2"), c3("TST", "C3");
	a1.setElementSymbol("S");
	a2.setElementSymbol("H");
	a3.setElementSymbol("H");

	b1.setElementSymbol("H");
	b2.setElementSymbol("H");

	c1.setElementSymbol("H");
	c2.setElementSymbol("H");
	c3.setElementSymbol("S");

	c.setElementSymbol("C");
	b.setElementSymbol("C");
	a.setElementSymbol("C");
	
	AtomGroup group;
	group += &a; group += &a1; group += &a2; group += &a3;
	group += &b; group += &b1; group += &b2;
	group += &c; group += &c1; group += &c2; group += &c3;

	GeometryTable table;
	table.addGeometryLength("TST", "A", "A1", 1.5, 0);
	table.addGeometryLength("TST", "A", "A2", 1.5, 0);
	table.addGeometryLength("TST", "A", "A3", 1.5, 0);

	table.addGeometryLength("TST", "B", "B1", 1.5, 0);
	table.addGeometryLength("TST", "B", "B2", 1.5, 0);

	table.addGeometryLength("TST", "C", "C1", 1.5, 0);
	table.addGeometryLength("TST", "C", "C2", 1.5, 0);
	table.addGeometryLength("TST", "C", "C3", 1.5, 0);
	
	table.addGeometryLength("TST", "B", "A", 1.5, 0);
	table.addGeometryLength("TST", "B", "C", 1.5, 0);

	table.addGeometryAngle("TST", "A1", "A", "B", 109.5, 0);
	table.addGeometryAngle("TST", "A2", "A", "B", 109.5, 0);
	table.addGeometryAngle("TST", "A3", "A", "B", 109.5, 0);
	table.addGeometryAngle("TST", "A1", "A", "A2", 109.5, 0);
	table.addGeometryAngle("TST", "A2", "A", "A3", 109.5, 0);
	table.addGeometryAngle("TST", "A3", "A", "A1", 109.5, 0);

	table.addGeometryAngle("TST", "B1", "B", "A", 109.5, 0);
	table.addGeometryAngle("TST", "B2", "B", "A", 109.5, 0);
	table.addGeometryAngle("TST", "C", "B", "A", 109.5, 0);
	table.addGeometryAngle("TST", "B1", "B", "B2", 109.5, 0);
	table.addGeometryAngle("TST", "B2", "B", "C", 109.5, 0);
	table.addGeometryAngle("TST", "C", "B", "B1", 109.5, 0);

	table.addGeometryAngle("TST", "C1", "C", "B", 109.5, 0);
	table.addGeometryAngle("TST", "C2", "C", "B", 109.5, 0);
	table.addGeometryAngle("TST", "C3", "C", "B", 109.5, 0);
	table.addGeometryAngle("TST", "C1", "C", "C2", 109.5, 0);
	table.addGeometryAngle("TST", "C2", "C", "C3", 109.5, 0);
	table.addGeometryAngle("TST", "C3", "C", "C1", 109.5, 0);
	
	table.addGeometryTorsion("TST", "A1", "A", "B", "C", 35, 0, 1);
	table.addGeometryTorsion("TST", "A", "B", "C", "C1", 70, 0, 1);
	
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

	{
		BondTorsion torsion(nullptr, &a1, &a, &b, &c, 0);
		bad |= measurement_matches(torsion, 35);
	}

	{
		BondTorsion torsion(nullptr, &a, &b, &c, &c1, 0);
		bad |= measurement_matches(torsion, 70);
	}
	
	if (bad)
	{
		Grapher gr;
		gr.generateGraphs(group.possibleAnchor(0), UINT_MAX);
		int added = gr.atoms().size();
		
		std::cout << "Added " << added << " atoms." << std::endl;

		for (size_t i = 0; i < gr.graphCount(); i++)
		{
			std::cout << gr.graph(i)->desc();
		}
	}
	
	return bad;
}
