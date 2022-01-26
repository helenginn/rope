#include <sstream>
#include "matrix_functions.h"
#include "Atom.h"
#include "BondLength.h"
#include "BondAngle.h"
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
	trim(_ele);
}

void Atom::setAtomName(std::string name)
{
	_atomName = name;
	to_upper(_atomName);
	trim(_atomName);
}

void Atom::setAtomNum(int num)
{
	if (num < 0)
	{
		throw std::runtime_error("Attempting to assign negative atom number");
	}

	_atomNum = num;
}

void Atom::setDerivedPosition(glm::vec3 &pos)
{
	_derived.pos = pos;
	changedPosition();
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
		_changedPosition = false;
		unlockMutex();
		return true;
	}

	return false;
}

void Atom::setCode(std::string code)
{
	to_upper(code);
	_code = code;
}

Atom *Atom::connectedAtom(int i)
{
	return bondLength(i)->otherAtom(this);
}

glm::mat4x4 Atom::coordinationMatrix(Atom *children[4], int count, Atom *prev)
{
	glm::mat4x4 ret = glm::mat4(0.f);

	float lengths[4] = {0, 0, 0, 0};
	float angles[5] = {0, 0, 0, 0, 0};
	
	for (size_t i = 0; i < count; i++)
	{
		Atom *a = children[i];
		BondLength *bond = findBondLength(this, a);
		lengths[i] = bond->length();
	}

	if (count >= 2)
	{
		angles[0] = findBondAngle(children[0], this, children[1])->angle();
	}

	if (count >= 3)
	{
		angles[1] = findBondAngle(children[1], this, children[2])->angle();
		angles[2] = findBondAngle(children[2], this, children[0])->angle();
	}

	if (count == 4)
	{
		angles[3] = findBondAngle(children[1], this, children[3])->angle();
		angles[4] = findBondAngle(children[0], this, children[3])->angle();

		insert_four_atoms(ret, lengths, angles);
	}

	else if (prev == nullptr)
	{
		if (count == 3)
		{
			insert_three_atoms(ret, lengths, angles);
		}
		else if (count == 2)
		{
			insert_two_atoms(ret, lengths, angles[0]);
		}
		else if (count == 1)
		{
			insert_one_atom(ret, lengths[0]);
		}
	}
	else if (prev != nullptr)
	{
		glm::mat4x4 tmp = glm::mat4(0.f);

		/* make room in length/angle array for "prev" */
		for (size_t i = 3; i > 0; i--)
		{
			lengths[i] = lengths[i - 1];
		}

		lengths[0] = findBondLength(this, prev)->length();

		/* angle prev-to-0 */
		if (count >= 1)
		{
			angles[0] = findBondAngle(prev, this, children[0])->angle();
		}
		
		/* angle prev-to-1 and 1-to-0 */
		if (count >= 2)
		{
			angles[1] = findBondAngle(children[0], this, children[1])->angle();
			angles[2] = findBondAngle(prev, this, children[1])->angle();
		}

		/* angle 2-to-1 and prev-to-2 */
		if (count == 3)
		{
			angles[3] = findBondAngle(children[2], this, children[1])->angle();
			angles[4] = findBondAngle(children[2], this, prev)->angle();
		}

		if (count == 3)
		{
			insert_four_atoms(tmp, lengths, angles);
		}
		else if (count == 2)
		{
			insert_three_atoms(tmp, lengths, angles);
		}
		else if (count == 1)
		{
			insert_two_atoms(tmp, lengths, angles[0]);
		}

		if (count == 2)
		{
			std::cout << "VALUES" << std::endl;
			std::cout << "I am: " << atomName() << ", caller " <<
			prev->atomName() << std::endl;

			for (size_t i = 0; i < count; i++)
			{
				std::cout << children[i]->atomName() << " ";
			}
			std::cout << std::endl;


			for (size_t i = 0; i < 4; i++)
			{
				std::cout << "\tLength " << i << " = " << lengths[i] << std::endl;
				std::cout << "\tAngle " << i << " = " << angles[i] << std::endl;
			}

			std::cout << "\tAngle " << 4 << " = " << angles[4] << std::endl;
			std::cout << std::endl;
			std::cout << "Tmp is: " << glm::to_string(tmp) << std::endl;
			std::cout << std::endl;
		}
		
		for (size_t i = 0; i < 3; i++)
		{
			/* now remove the prev bond as we're not going backwards */
			ret[i] = tmp[i + 1];
		}
	}

	for (size_t i = 0; i < 4; i++)
	{
		ret[i][3] += 1;
	}

	for (size_t i = 0; i < 4; i++)
	{
		if (!is_glm_vec_sane(ret[i]))
		{
			std::ostringstream ss;
			ss << atomName() << " produced insane coordinate matrix: ";
			ss << std::endl;
			ss << glm::to_string(ret) << std::endl;
			throw std::runtime_error(ss.str());
		}
	}
	if (count > 2 && false)
	{
		ret[0] = glm::vec4(1., 0, 0.5, 1);
		ret[1] = glm::vec4(0, 1., 0.5, 1);
		ret[2] = glm::vec4(-1, 0, 0.5, 1);
		ret[3] = glm::vec4(0, -1, 0.5, 1);
	}
	
//	std::cout << glm::to_string(ret) << std::endl;
	
	return ret;
}

