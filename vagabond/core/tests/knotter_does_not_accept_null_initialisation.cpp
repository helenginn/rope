#include "../Knotter.h"

int main()
{
	try
	{
		Knotter knotter(nullptr, nullptr);
	}
	catch (std::runtime_error err)
	{
		return 0;
	}
	
	return 1;
}
