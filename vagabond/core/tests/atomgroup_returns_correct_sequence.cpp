#include <iostream>
#include "../matrix_functions.h"
#include "../BondTorsion.h"
#include "../Sequence.h"
#include "../Grapher.h"
#include "../Knotter.h"
#include "../AtomGroup.h"
#include "../GeometryTable.h"
#include "../Atom.h"

int main()
{
	Atom a("AAA", "A");
	Atom b("BBB", "B");
	Atom c("CCC", "C");

	a.setResidueNumber(1);
	b.setResidueNumber(2);
	c.setResidueNumber(3);
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
	
	return !(result == "AAA BBB CCC");
}

