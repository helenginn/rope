#include "Atom.h"

Atom::Atom()
{
	_setupInitial = false;
}

void Atom::setInitialPosition(glm::vec3 pos, float b, glm::mat3x3 tensor)
{
	_initial.pos = pos;
	_initial.b = b;
	_initial.tensor = tensor;
	
	if (!_setupInitial)
	{
		_derived.pos = pos;
		_derived.b = b;
		_derived.tensor = tensor;
		_setupInitial = true;
	}
}

void Atom::setElementSymbol(std::string ele)
{

}
