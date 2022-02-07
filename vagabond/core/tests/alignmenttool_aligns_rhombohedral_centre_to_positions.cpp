#include <iostream>
#include "../Knotter.h"
#include "../AtomGroup.h"
#include "../GeometryTable.h"
#include "../Atom.h"
#include "../AlignmentTool.h"

int main()
{
	Atom a("TST", "A"), a1("TST", "A1"), a2("TST", "A2"), a3("TST", "A3");
	a1.setElementSymbol("H");
	a2.setElementSymbol("H");
	a3.setElementSymbol("H");
	a.setElementSymbol("C");
	a.setInitialPosition(glm::vec3(5., 5., 5.), 30);
	a1.setInitialPosition(glm::vec3(5., 4.5, 5.), 30);
	a2.setInitialPosition(glm::vec3(5., 5., 4.5), 30);
	a3.setInitialPosition(glm::vec3(4.5, 5., 5.), 30);
	
	AtomGroup group;
	group += &a; group += &a1; group += &a2; group += &a3;
	
	GeometryTable table;
	table.addGeometryLength("TST", "A", "A1", 0.5, 0);
	table.addGeometryLength("TST", "A", "A2", 0.5, 0);
	table.addGeometryLength("TST", "A", "A3", 0.5, 0);

	table.addGeometryAngle("TST", "A1", "A", "A2", 90.0, 0);
	table.addGeometryAngle("TST", "A2", "A", "A3", 90.0, 0);
	table.addGeometryAngle("TST", "A3", "A", "A1", 90.0, 0);
	
	Knotter knotter(&group, &table);
	knotter.knot();

	AlignmentTool tool(&group);
	tool.run();
	bool bad = false;
	
	for (size_t i = 0; i < group.size(); i++)
	{
		glm::vec3 empty = glm::vec3(0.);
		group[i]->setDerivedPosition(empty);
	}
	
	group.recalculate();

	for (size_t i = 0; i < group.size(); i++)
	{
		Atom *a = group[i];
		
		glm::vec3 pos = a->derivedPosition();
		glm::vec3 init = a->initialPosition();
		
		double l = glm::length(pos - init);
		
		if (l > 1e-3)
		{
			std::cout << a->atomName() << " does not match." << std::endl;
			std::cout << glm::to_string(init) << " vs ";
			std::cout << glm::to_string(pos) << std::endl;
			bad = true;
		}
	}
	
	return bad;
}
