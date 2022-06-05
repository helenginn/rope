#include "../Sequence.h"
#include "../Entity.h"
#include "../../c4x/DataGroup.h"

int main()
{
	std::string mseq("ITHINKCOWSAREGREAT");
	std::string lseq("ITHOUGHTCOWSWEREGREAT");
	Sequence master(mseq);
	Sequence local(lseq);

	Entity entity;
	entity.setName("present");
	entity.setSequence(&master);
	
	SequenceComparison *sc = master.newComparison(&entity);
	local.mapFromMaster(sc);
	DataGroup<float>::Array vals;
	entity.sequence()->torsionsFromMapped(&local, vals, false, 10);
	
	if (vals.size() != 18)
	{
		std::cout << "Vals size: " << vals.size() << " not 18" << std::endl;
		return 1;
	}

	for (size_t i = 0; i < vals.size(); i++)
	{
		if (fabs(vals[i] - 10) >= 1e-6)
		{
			std::cout << "Wrong value: " << vals[i] << std::endl;
			return 1;
		}
	}
}
