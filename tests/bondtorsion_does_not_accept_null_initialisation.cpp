#include "../src/BondTorsion.h"

int main()
{
	try
	{
		BondTorsion(nullptr, nullptr, nullptr, nullptr, nullptr, 2);

	}
	catch (std::runtime_error err)
	{
		return 0;
	}

	return 1;
}
