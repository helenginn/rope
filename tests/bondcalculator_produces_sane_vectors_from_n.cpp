#include "../src/Cif2Geometry.h"
#include "../src/AtomGroup.h"
#include "../src/Atom.h"
#include "../src/BondCalculator.h"
#include "../src/matrix_functions.h"
#include <iostream>

#include "bondcalculator_produces_sane_vectors.h"

int main()
{
	return check_vecs("N");
}
