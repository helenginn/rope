#include "Atom.h"
#include "FileReader.h"

Atom::Atom()
{
	_setupInitial = false;
	_hetatm = false;
	_occupancy = 1.;
	_residueNumber = 1;
	_atomNum = 1;
	_changedPosition = false;
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
	
	changedPosition();
}

void Atom::setElementSymbol(std::string ele)
{
	if (ele.length() == 0)
	{
		throw std::runtime_error("Attempting to use empty element assignment");
	}
	if (!is_str_alphabetical(ele))
	{
		throw std::runtime_error("Attempting to use non-alphabetical element "
		                         + ele);
	}
	_ele = ele;
	to_upper(_ele);
}

void Atom::setAtomName(std::string name)
{
	_atomName = name;
	to_upper(_atomName);
}

void Atom::setAtomNum(int num)
{
	if (num < 0)
	{
		throw std::runtime_error("Attempting to assign negative atom number");
	}

	_atomNum = num;
}

void Atom::changedPosition()
{
	lockMutex();
	_changedPosition = true;
	unlockMutex();
}

bool Atom::positionChanged()
{
	bool tmp = false;
	if (_mutex.try_lock())
	{
		tmp = _changedPosition;
		unlockMutex();
	}

	return tmp;
}

bool Atom::fishPosition(glm::vec3 *p)
{
	if (_mutex.try_lock())
	{
		*p = _derived.pos;
		unlockMutex();
		return true;
	}

	return false;
}
