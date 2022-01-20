#include "../src/Renderable.h"

int main()
{
	Renderable *r = new Renderable();
	
	double radius = r->envelopeRadius();
	
	return !(radius > -0.001 && radius < 0.001);
}
