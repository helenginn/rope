#include <iostream>
#include "../Grapher.h"
#include "../Knotter.h"
#include "../AtomGroup.h"
#include "../GeometryTable.h"
#include "../Atom.h"

int main()
{
	Atom a("TST", "A");
	Atom b("TST", "B");
	Atom c("TST", "C");
	Atom d("TST", "D");

	AtomGroup group;
	group += &a; group += &b; group += &c; group += &d;

	GeometryTable table;
	table.addGeometryLength(".", "B", "A", 1.5, 0);
	table.addGeometryLength(".", "B", "C", 1.5, 0);
	table.addGeometryLength(".", "B", "D", 1.5, 0);

	table.addGeometryAngle(".", "C", "B", "A", 109.5, 0);
	table.addGeometryAngle(".", "D", "B", "A", 109.5, 0);
	table.addGeometryAngle(".", "D", "B", "C", 109.5, 0);

	Knotter knotter(&group, &table);
	knotter.knot();
	
	Grapher gr;
	gr.generateGraphs(&a);
	gr.assignMainChain();
	
	int count = 0;
	for (size_t i = 0; i < group.size(); i++)
	{
		if (group[i]->isMainChain())
		{
			count++;
		}
	}
	
	std::cout << "Count: " << count << std::endl;
	
	return !(count == 3);
}
