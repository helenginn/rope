#include "../CifFile.h"
#include "../AtomGroup.h"
#include "../BondSequence.h"

int main()
{
	std::string path = "/assets/geometry/CHX.cif";

	CifFile geom = CifFile(path);
	geom.setAutomaticKnot(true);
	geom.parse();
	
	AtomGroup *atoms = geom.atoms();
	
	bool bad = false;

	for (size_t i = 0; i < atoms->possibleAnchorCount(); i++)
	{
		Grapher gr;
		gr.generateGraphs(atoms->possibleAnchor(i));
		gr.calculateMissingMaxDepths();

		int added = gr.graphCount();

		for (size_t i = 0; i < added; i++)
		{
			bool result = gr.graph(i)->checkAtomGraph();
			
			if (result != 0)
			{
				std::cout << "Problem with block " << i << std::endl;
				bad = true;
			}
		}
	}
	
	delete atoms;

	return bad;
}

