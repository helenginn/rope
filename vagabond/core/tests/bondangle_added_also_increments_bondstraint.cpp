#include "../BondAngle.h"
#include "../AtomGroup.h"
#include "../Atom.h"

int main()
{
	AtomGroup g;
	Atom a, b, c;
	g += &a;
	g += &b;
	g += &c;
	
	new BondAngle(&g, &a, &b, &c, 2);
	
	if (g.bondAngleCount() != 1 || g.bondstraintCount() != 1)
	{
		return 1;
	}

	return 0;
}
