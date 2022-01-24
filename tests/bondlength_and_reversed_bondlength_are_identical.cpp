#include "../src/BondLength.h"
#include "../src/AtomGroup.h"
#include "../src/Atom.h"
#include <iostream>

int main()
{
	AtomGroup g;
	Atom a, b;
	g += &a;
	g += &b;
	
	BondLength *length = new BondLength(&g, &a, &b, 2);
	BondLength *reversed = new BondLength(&g, &b, &a, 2);
	
	return !(*length == *reversed);
}



