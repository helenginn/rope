#include "../FileReader.h"

int main()
{
	std::string full = ",split,me";
	std::vector<std::string> list = split(full, ',');

	if (list.size() != 3)
	{
		return 1;
	}
	
	if (list[0] == "")
	{
		return 0;
	}
	
	return 1;
}

