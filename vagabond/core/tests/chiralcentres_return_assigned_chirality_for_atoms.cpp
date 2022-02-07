#include "../GeometryTable.h"
#include "../AtomGroup.h"
#include "../Atom.h"
#include <iostream>

int main()
{
	AtomGroup g;
	Atom centre;
	centre.setAtomName("CN");
	centre.setCode("TST");

	Atom a, b, c;
	a.setAtomName("A");
	a.setCode("TST");

	b.setAtomName("B");
	b.setCode("TST");

	c.setAtomName("C");
	c.setCode("TST");

	g += &centre;
	g += &a;
	g += &b;
	g += &c;
	
	GeometryTable table;
	
	table.addGeometryChiral("TST", "CN", "A", "B", "C", 1);
	
	int pos[3];
	int neg[3];

	pos[0] = table.chirality("TST", "CN", "A", "B", "C");
	pos[1] = table.chirality("TST", "CN", "C", "A", "B");
	pos[2] = table.chirality("TST", "CN", "B", "C", "A");

	neg[0] = table.chirality("TST", "CN", "B", "A", "C");
	neg[1] = table.chirality("TST", "CN", "A", "C", "B");
	neg[2] = table.chirality("TST", "CN", "C", "B", "A");
	
	for (size_t i = 0; i < 3; i++)
	{
		if (pos[i] <= 0 || neg[i] >= 0)
		{
			std::cout << "Incorrect chirality assignment" << std::endl;
			return 1;
		}
	}

	return 0;
}
