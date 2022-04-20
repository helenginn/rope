#include "../CifFile.h"
#include "../AtomGroup.h"
#include "../BondSequence.h"

int main()
{
	std::string path = "/assets/geometry/GLY.cif";

	CifFile geom = CifFile(path);
	geom.setAutomaticKnot(true);
	geom.parse();
	
	AtomGroup *atoms = geom.atoms();
	Atom *anchor = atoms->firstAtomWithName("OXT");

	Grapher gr;
	gr.generateGraphs(anchor);
	gr.calculateMissingMaxDepths();
	
	for (size_t i = 0; i < gr.graphCount(); i++)
	{
		int diff = gr.remainingDepth(i);
		if (diff < 0)
		{
			gr.graph(i)->checkAtomGraph();
			return 1;
		}
	}

	return 0;
}



