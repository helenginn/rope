#include "../BondLength.h"
#include "../AtomGroup.h"
#include "../Atom.h"

int main()
{
	AtomGroup g;
	Atom a, b;
	
	try
	{
		BondLength(&g, &a, &b, 2);
	}
	catch (std::runtime_error err)
	{
		return 0;
	}
	

	return 1;
}


