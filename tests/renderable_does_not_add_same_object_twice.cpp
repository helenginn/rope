#include "../src/Renderable.h"

int main()
{
	Renderable *r = new Renderable();
	Renderable *child = new Renderable();
	
	try
	{
		r->addObject(child);
		r->addObject(child);
	}
	catch (std::runtime_error err)
	{
		return 0;
	}
	
	return 1;
}


