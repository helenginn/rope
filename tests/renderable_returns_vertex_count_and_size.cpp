#include "../src/Renderable.h"

int main()
{
	Renderable *r = new Renderable();
	r->addVertex(glm::vec3(0, 0, 0));
	
	if (r->vertexCount() != 1)
	{
		throw(std::runtime_error("Vertex count is not one "
		                         "after adding single vertex"));
	}
	
	if (r->vSize() != sizeof(Vertex))
	{
		throw(std::runtime_error("Vertex size is not equal to one times "
		                         "size of single vertex"));
	}
	
	return 0;
}


