#include "../Knotter.h"
#include "../GeometryTable.h"
#include "../AtomGroup.h"
#include "../Atom.h"

int main()
{
	AtomGroup group;
	GeometryTable table;
	Knotter k(&group, &table);
	k.knot();

	return 0;
}
