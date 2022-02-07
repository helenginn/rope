#include "../BondSequenceHandler.h"
#include "../Atom.h"
#include "../AtomGroup.h"
#include "../GeometryTable.h"
#include "../Knotter.h"

int main()
{
	Atom a("TST", "A"), a1("TST", "A1"), a2("TST", "A2"), a3("TST", "A3");
	a1.setElementSymbol("H");
	a2.setElementSymbol("H");
	a3.setElementSymbol("H");
	a.setElementSymbol("C");
	a.setInitialPosition(glm::vec3(5., 5., 5.), 30);
	a1.setInitialPosition(glm::vec3(5., 4.5, 5.), 30);
	a2.setInitialPosition(glm::vec3(5., 5., 4.5), 30);
	a3.setInitialPosition(glm::vec3(4.5, 5., 5.), 30);
	
	AtomGroup group;
	group += &a; group += &a1; group += &a2; group += &a3;
	
	GeometryTable table;
	table.addGeometryLength("TST", "A", "A1", 0.5, 0);
	table.addGeometryLength("TST", "A", "A2", 0.5, 0);
	table.addGeometryLength("TST", "A", "A3", 0.5, 0);

	table.addGeometryAngle("TST", "A1", "A", "A2", 90.0, 0);
	table.addGeometryAngle("TST", "A2", "A", "A3", 90.0, 0);
	table.addGeometryAngle("TST", "A3", "A", "A1", 90.0, 0);
	
	Knotter knotter(&group, &table);
	knotter.knot();

	BondSequenceHandler *handler = new BondSequenceHandler();
	handler->setTotalSamples(1);
	handler->setMaxThreads(1);
	handler->setIgnoreHydrogens(true);
	BondCalculator::AnchorExtension ext = {&a, UINT_MAX};
	handler->addAnchorExtension(ext);
	handler->setup();

	int added = handler->sequence(0)->addedAtomsCount();
	
	if (added != 1)
	{
		std::cout << "Did not ignore hydrogens when asked, " << added
		<< " atoms." << std::endl;

		return 1;
	}

	return 0;
}
