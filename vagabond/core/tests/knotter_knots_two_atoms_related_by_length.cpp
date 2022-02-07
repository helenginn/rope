#include <iostream>
#include "../Knotter.h"
#include "../GeometryTable.h"
#include "../BondLength.h"
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
	b.setInitialPosition(glm::vec3(0, 0, 1.1), 30);
	group += &a;
	group += &b;
	
	GeometryTable table;
	table.addGeometryLength("COD", "B", "A", 1., 0.01);
	
	Knotter k(&group, &table);
	k.knot();
	
	if (a.bondLengthCount() != 1 || b.bondLengthCount() != 1)
	{
		return 1;
	}

	int l1 = a.bondLength(0)->length();
	int l2 = b.bondLength(0)->length();
	
	l1 -= 1;
	l2 -= 1;
	
	return !(fabs(l1) + fabs(l2) < 1e-6);
}

