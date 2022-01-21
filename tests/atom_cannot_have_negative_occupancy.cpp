#include "../src/Atom.h"

int main()
{
	Atom *atom = new Atom();
	
	atom->setOccupancy(-1);
	return (atom->occupancy() < 0);
}


