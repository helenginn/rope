#include "../CifFile.h"
#include "../AtomGroup.h"
#include "../BondAngle.h"
#include "../BondLength.h"
#include "../BondCalculator.h"
#include "bondcalculator_checks_geometry_for_ligand.h"

int main()
{
	std::string path = "/assets/geometry/PO4.cif";

	return checkGeometry(path, 0.1);
}

