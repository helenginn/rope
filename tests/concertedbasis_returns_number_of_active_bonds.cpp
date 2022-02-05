#include "../src/ConcertedBasis.h"
#include "../src/BondTorsion.h"
#include "../src/Atom.h"

int main()
{
	Atom a, b, c, d;
	BondTorsion *ts[5];
	
	ConcertedBasis cb;

	for (size_t i = 0; i < 5; i++)
	{
		ts[i] = new BondTorsion(nullptr, &a, &b, &c, &d, 2);
		ts[i]->setRefinedAngle(100);
		cb.addTorsion(ts[i]);
	}
	
	std::vector<bool> mask = {true, true, false, true, false};

	cb.supplyMask(mask);
	cb.prepare();
	
	size_t num = cb.activeBonds();

	return !(num == 3);
}
