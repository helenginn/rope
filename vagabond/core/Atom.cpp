#include <sstream>
#include "matrix_functions.h"
#include "Atom.h"
#include "BondLength.h"
#include "BondAngle.h"
#include "Chirality.h"
#include "../utils/FileReader.h"

Atom::Atom()
{

}

Atom::Atom(std::string code, std::string name)
{
	_code = code;
	_atomName = name;
}

void Atom::setInitialPosition(glm::vec3 pos, float b, glm::mat3x3 tensor)
{
	_initial.pos.ave = pos;
	if (b > 0)
	{
		_initial.b = b;
	}
	_initial.tensor = tensor;
	
	if (!_setupInitial)
	{
		_derived.pos.ave = pos;
		if (b > 0)
		{
			_derived.b = b;
		}
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
	lockMutex();
	_derived.pos.ave = pos;
	unlockMutex();
	changedPosition();
}

void Atom::setDerivedPositions(WithPos &pos)
{
	lockMutex();
	_derived.pos = pos;
	unlockMutex();
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

bool Atom::fishPositions(WithPos *wp)
{
	if (_mutex.try_lock())
	{
		*wp = _derived.pos;
		_changedPosition = false;
		unlockMutex();
		return true;
	}

	return false;

}

bool Atom::fishPosition(glm::vec3 *p)
{
	if (_mutex.try_lock())
	{
		*p = _derived.pos.ave;
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

void Atom::checkChirality(glm::mat4x4 &ret, Atom *prev, 
                          Atom *children[4], const int count)
{
	if (chiralityCount() == 0 || (!prev && count < 3) || (prev && count < 2))
	{
		return;
	}

	bool found = false;
	for (size_t m = 0; m < chiralityCount(); m++)
	{
		Chirality *ch = chirality(m);
		Atom *chosen[4] = {nullptr, nullptr, nullptr, nullptr};
		Atom *index[4] = {nullptr, nullptr, nullptr, nullptr};
		int offset = (prev != nullptr ? 1 : 0);

		if (prev != nullptr)
		{
			chosen[0] = prev;
			index[0] = prev;
		}

		for (size_t i = 0; i < count; i++)
		{
			chosen[i + offset] = children[i];
			index[i + offset] = children[i];
		}

		int sign = ch->get_sign(&chosen[0], &chosen[1], &chosen[2], &chosen[3]);

		if (sign == 0)
		{
			continue;
		}

		int n = 0;
		glm::vec3 vecs[3];

		for (size_t i = 0; i < 4; i++)
		{
			if (chosen[i] == nullptr)
			{
				continue;
			}

			for (size_t j = 0; j < 4; j++)
			{
				if (index[j] == chosen[i])
				{
					vecs[n] = glm::vec3(ret[j]);
					n++;
				}
			}
		}

		if (n != 3)
		{
			continue;
		}

		found = true;
		glm::vec3 cr = glm::cross(vecs[1], vecs[2]);
		double dot = glm::dot(cr, vecs[0]);

		if (dot * sign < 0) // flippity flip
		{
			for (size_t i = 0; i < 4; i++)
			{
				ret[i].y *= -1;
			}
		}

		return;
	}
	
	if (!found)
	{
		if (prev)
		{
			std::cout << "Prev: " << prev->desc() << std::endl;
		}
		for (size_t i = 0; i < 4; i++)
		{
			if (children[i])
			{
				std::cout << children[i]->desc() << std::endl;
			}
		}
		std::cout << "... against..." << std::endl;
		for (size_t i = 0; i < chiralityCount(); i++)
		{
			std::cout << chirality(i)->desc() << std::endl;
		}

		throw (std::runtime_error("Couldn't match any chirality measurements"));
	}
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
		angles[0] = findBondAngle(children[0], this, children[1], true)->angle();
	}

	if (count >= 3)
	{
		angles[1] = findBondAngle(children[1], this, children[2], true)->angle();
		angles[2] = findBondAngle(children[2], this, children[0], true)->angle();
	}

	if (count == 4)
	{
		angles[3] = findBondAngle(children[1], this, children[3], true)->angle();
		angles[4] = findBondAngle(children[0], this, children[3], true)->angle();

		insert_four_atoms(ret, lengths, angles);
		checkChirality(ret, prev, children, count);
	}

	else if (prev == nullptr)
	{
		if (count == 3)
		{
			insert_three_atoms(ret, lengths, angles);
			checkChirality(ret, prev, children, count);
		}
		else if (count == 2)
		{
			insert_two_atoms(ret, lengths, angles[0]);
		}
		else if (count == 1)
		{
			/* why add two atoms if only one is present? if this is the anchor,
			 * such as O in a C=O carbonyl bond, the second length must be used 
			 * to calculate the torsion, or we'll calculate a torsion value 
			 * of nan */
			lengths[1] = 1;
			insert_two_atoms(ret, lengths, 90);
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
			angles[0] = findBondAngle(prev, this, children[0], true)->angle();
		}
		
		/* angle prev-to-1 and 1-to-0 */
		if (count >= 2)
		{
			angles[1] = findBondAngle(children[0], this, 
			                          children[1], true)->angle();
			angles[2] = findBondAngle(prev, this, children[1], true)->angle();
		}

		/* angle 2-to-1 and prev-to-2 */
		if (count == 3)
		{
			angles[3] = findBondAngle(children[2], this, 
			                          children[1], true)->angle();
			angles[4] = findBondAngle(children[2], this, prev, true)->angle();
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

		checkChirality(tmp, prev, children, count);
		
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
	
	return ret;
}


void Atom::setTransformation(glm::mat4x4 transform)
{
	_transform = transform * _transform;
}

const std::string Atom::desc() const
{
	std::string str;
	if (_chain.length())
	{
		str += _chain + "-";
	}
	str +=  code() + i_to_str(residueNumber()) + ":" + atomName();
	return str;
}
