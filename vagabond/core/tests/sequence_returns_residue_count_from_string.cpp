#include "../Sequence.h"
#include "../SequenceComparison.h"

int main()
{
	std::string cows = "ILIKECOWS";
	Sequence s(cows);

	int count = s.size();
	std::cout << "Count: " << count << std::endl;

	return !(count == cows.length());
}

