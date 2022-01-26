#include "../src/BondSequenceHandler.h"

int main()
{
	BondSequenceHandler *sh = new BondSequenceHandler(nullptr);
	sh->setMaxThreads(5);
	sh->setTotalSamples(1000);
	sh->setup();
	
	int thr = sh->threadCount();

	return !(thr == 5);
}
