#include "../BondLength.h"

int main()
{
	try
	{
		BondLength length(nullptr, nullptr, nullptr, 1);
	}
	catch (std::runtime_error err)
	{
		return 0;
	}
	
	return 1;
}

