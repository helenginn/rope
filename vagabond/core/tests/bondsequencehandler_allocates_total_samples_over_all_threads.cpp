#include "../BondSequenceHandler.h"
#include "../BondSequence.h"
#include <iostream>

int main()
{
	for (size_t i = 1; i < 6; i++)
	{
		BondSequenceHandler *handler = new BondSequenceHandler();
		handler->setTotalSamples(100);
		handler->setMaxThreads(i);
		handler->setup();
		
		int total = 0;
		for (size_t j = 0; j < handler->sequenceCount(); j++)
		{
			const BondSequence *sequence = handler->sequence(j);
			total += sequence->sampleCount();
		}
		
		if (total != (i + 2) * 100)
		{
			std::cout << "Total " << total << " instead of 100" << std::endl;
			return 1;
		}
		
		delete handler;
	}
	
	return 0;
}
