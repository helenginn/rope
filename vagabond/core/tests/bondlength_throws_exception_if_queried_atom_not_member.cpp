#include "../BondLength.h"
#include "../AtomGroup.h"
#include "../Atom.h"

int main()
{
	AtomGroup g;
	Atom a, b, c;
	g += &a;
	g += &b;
	g += &c;
	
	BondLength *length = new BondLength(&g, &a, &b, 2);
	
	try
	{
		length->otherAtom(&c);
	}
	catch (std::runtime_error err)
	{
		return 0;
	}

	return 1;
}
