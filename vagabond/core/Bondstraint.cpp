#include <Bondstraint.h>
#include <Atom.h>

const bool Bondstraint::hasHydrogen() const
{
	Key k = key(0);

	for (size_t i = 0; i < 4; i++)
	{	
		if (k.atoms[i]->elementSymbol() == "H")
		{
			return true;
		}
	}
	
	return false;
}
