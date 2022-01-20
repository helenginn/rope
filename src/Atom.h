#ifndef __vagabond__Atom__
#define __vagabond__Atom__

#include "glm_import.h"

class Atom
{
public:
	Atom();

	struct AtomPlacement
	{
		glm::mat3x3 tensor;
		float b;
		glm::vec3 pos;
	}; 
	
	void setInitialPosition(glm::vec3 pos, float b, 
	                        glm::mat3x3 tensor = glm::mat3(1.f));
	
	const float &initialBFactor()
	{
		return _initial.b;
	}
	
	const glm::vec3 &initialPosition()
	{
		return _initial.pos;
	}
	
	const float &derivedBFactor()
	{
		return _derived.b;
	}
	
	const glm::vec3 &derivedPosition()
	{
		return _derived.pos;
	}
	
	void setElementSymbol(std::string ele);
	
	std::string elementSymbol()
	{
		return _ele;
	}
private:
	AtomPlacement _initial;
	AtomPlacement _derived;

	bool _setupInitial;
	std::string _ele;
};

#endif
