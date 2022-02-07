#include "../Cif2Geometry.h"
#include "../AtomGroup.h"
#include "../Atom.h"
#include "../BondCalculator.h"
#include "../matrix_functions.h"
#include <iostream>

#include "bondcalculator_produces_sane_vectors.h"

int main()
{
	return check_vecs("OXT");
}

