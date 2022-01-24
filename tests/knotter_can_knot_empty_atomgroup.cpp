#include "../src/Knotter.h"
#include "../src/GeometryTable.h"
#include "../src/AtomGroup.h"
#include "../src/Atom.h"

int main()
{
	AtomGroup group;
	GeometryTable table;
	Knotter k(&group, &table);
	k.knot();

	return 0;
}
