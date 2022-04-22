#include <iostream>
#include "../Sequence.h"
#include "../Grapher.h"
#include "../Knotter.h"
#include "../AtomGroup.h"
#include "../GeometryTable.h"
#include "../Atom.h"

int main()
{
	Atom a("LEU", "A");
	Atom b("THR", "B");
	Atom c("ARG", "C");

	a.setResidueId("1");
	b.setResidueId("2");
	c.setResidueId("3");
	c.setElementSymbol("C");
	b.setElementSymbol("C");
	a.setElementSymbol("C");
	
	AtomGroup group;
	group += &a;
	group += &b;
	group += &c;

	GeometryTable table;
	table.addGeometryLength(".", "B", "A", 1.5, 0, true);
	table.addGeometryLength(".", "B", "C", 1.5, 0, true);

	table.addGeometryAngle(".", "C", "B", "A", 109.5, 0, true);
	
	Knotter knotter(&group, &table);
	knotter.knot();
	
	Sequence *seq = group.sequence();
	std::string result = seq->str();
	
	return !(result == "LTR");
}

