#include "../Sequence.h"
#include "../Entity.h"

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
	
	Residue *test = entity.sequence()->residue(6);
	std::cout << "Testing for " << test << " from sequence " << &master << std::endl;
	Residue *corresponding = local.local_residue(test);
	
	if (!corresponding)
	{
		std::cout << "Corresponding is NULL" << std::endl;
		return 1;
	}

	if (corresponding->code() != "CYS")
	{
		std::cout << "Testing code: " << test->code() << std::endl;
		std::cout << "Corresponding code: " << corresponding->code() << std::endl;
		return 1;
	}
	
	std::cout << "Matched: " << corresponding->id().as_string() << std::endl;
}
