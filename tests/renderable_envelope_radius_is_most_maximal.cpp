#include "../src/Renderable.h"

int main()
{
	Renderable *r = new Renderable();
	
	r->addVertex(glm::vec3(0, 0, 0));
	r->addVertex(glm::vec3(0, 0, 0.5));
	r->addVertex(glm::vec3(0, 0, 1.0));
	
	double radius = r->envelopeRadius();
	
	return !(radius > 0.499 && radius < 0.501);
}
