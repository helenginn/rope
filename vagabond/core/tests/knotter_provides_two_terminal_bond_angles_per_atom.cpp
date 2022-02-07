#include "../Knotter.h"
#include "../GeometryTable.h"
#include "../BondAngle.h"
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
	group += &a;
	group += &b;
	group += &c;
	
	GeometryTable table;
	table.addGeometryLength("COD", "B", "A", 1., 0.01);
	table.addGeometryLength("COD", "B", "C", 1.5, 0.01);
	table.addGeometryLength("COD", "C", "A", 1., 0.01);
	table.addGeometryAngle("COD", "C", "B", "A", 2., 0.01);
	table.addGeometryAngle("COD", "B", "A", "C", 2, 0.01);
	table.addGeometryAngle("COD", "A", "C", "B", 2., 0.01);
	
	Knotter k(&group, &table);
	k.knot();
	
	if (a.terminalBondAngleCount() != 2 || b.terminalBondAngleCount() != 2)
	{
		return 1;
	}

	return 0;
}




