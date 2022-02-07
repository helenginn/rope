#include "../Renderable.h"

int main()
{
	Renderable *r = new Renderable();
	
	r->addVertex(glm::vec3(0, 0, 0));
	r->addVertex(glm::vec3(0, 0, 0.5));
	r->addVertex(glm::vec3(0, 0, 1.0));
	
	glm::vec3 centre = r->centroid();
	
	bool ok = !(centre.z > 0.499 && centre.z < 0.501);
	ok |= !(centre.x > -0.001 && centre.x < 0.001);
	ok |= !(centre.y > -0.001 && centre.y < 0.001);
	
	return ok;
}

