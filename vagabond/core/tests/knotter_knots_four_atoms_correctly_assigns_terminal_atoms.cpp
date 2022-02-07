#include <iostream>
#include "../Knotter.h"
#include "../GeometryTable.h"
#include "../AtomGroup.h"
#include "../Atom.h"

int main()
{
	AtomGroup group;
	Atom a;
	a.setAtomName("A");
	a.setCode("COD");
	a.setInitialPosition(glm::vec3(0, 0, 0), 30);
	Atom b;
	b.setAtomName("B");
	b.setCode("COD");
	b.setInitialPosition(glm::vec3(0, 0, 0.8), 30);
	Atom c;
	c.setAtomName("C");
	c.setCode("COD");
	c.setInitialPosition(glm::vec3(0, 0.8, 0.), 30);
	Atom d;
	d.setAtomName("D");
	d.setCode("COD");
	d.setInitialPosition(glm::vec3(0, 1.6, 0.), 30);
	group += &a;
	group += &b;
	group += &c;
	group += &d;
	
	GeometryTable table;
	table.addGeometryLength("COD", "B", "A", 1., 0.01);
	table.addGeometryLength("COD", "B", "C", 1.5, 0.01);
	table.addGeometryLength("COD", "C", "D", 1., 0.01);
	table.addGeometryAngle("COD", "C", "B", "A", 2., 0.01);
	table.addGeometryAngle("COD", "B", "C", "D", 2, 0.01);
	
	Knotter k(&group, &table);
	k.knot();
	
	if (a.terminalTorsionCount() != 1 || b.terminalTorsionCount() != 0
	    || c.terminalTorsionCount() != 0 || d.terminalTorsionCount() != 1)
	{
		return 1;
	}

	return 0;
}





