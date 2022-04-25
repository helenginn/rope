#include "../Sequence.h"
#include "../SequenceComparison.h"

int main()
{
	Sequence s("ILIKECOWSTHATEATGRASS");
	Sequence t("ALLCOWSMAYEATGRASS");
	
	SequenceComparison sc(&s, &t);
	
	int expected = 21;
	int entryCount = sc.entryCount();
	
	std::cout << "Expected: " << expected << std::endl;
	std::cout << "Entry count: " << entryCount << std::endl;
	
	return !(entryCount == expected);
}
