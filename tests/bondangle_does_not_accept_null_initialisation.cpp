#include "../src/BondAngle.h"

int main()
{
	try
	{
		BondAngle angle(nullptr, nullptr, nullptr, nullptr, 1);
	}
	catch (std::runtime_error err)
	{
		return 0;
	}
	
	return 1;
}
