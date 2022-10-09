// vagabond
// Copyright (C) 2022 Helen Ginn
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
// 
// Please email: vagabond @ hginn.co.uk for more details.

#include "Residue.h"
#include <gemmi/polyheur.hpp>

Residue::Residue(ResidueId id, std::string code, std::string chain)
{
	_code = code;
	_chain = chain;
	_id = id;
}

const std::string Residue::one_letter_code() const
{
	return gemmi::one_letter_code(std::vector<std::string>(1, _code));
}

void Residue::addTorsionRef(TorsionRef &ref)
{
	_refs.insert(ref);
}

const std::string Residue::desc() const
{
	std::string name = _code + i_to_str(as_num());
	return name;
}

TorsionRef Residue::copyTorsionRef(const std::string &desc)
{
	std::set<TorsionRef>::iterator it = _refs.find(TorsionRef{desc});
	
	if (it == _refs.end())
	{
		return TorsionRef("");
	}
	
	TorsionRef ref = *it;
	return ref;
}

void Residue::replaceTorsionRef(TorsionRef &newRef)
{
	_refs.erase(newRef);
	_refs.insert(newRef);

}

bool Residue::supplyRefinedAngle(std::string desc, double angle)
{
	TorsionRef copy = copyTorsionRef(desc);
	if (!copy.valid())
	{
		return false;
	}

	copy.setRefinedAngle(angle);
	replaceTorsionRef(copy);
	return true;
}

const size_t Residue::torsionCount() const
{
	int count = 0;
	for (const TorsionRef &t : _refs)
	{
		if (t.coversMainChain())
		{
			count++;
		}
	}
	
	return count;
}

void Residue::housekeeping()
{
	_atomNames.clear();

	for (const TorsionRef &ref : _refs)
	{
		std::vector<std::string> comps = split(ref.desc(), '-');
		
		for (const std::string &comp : comps)
		{
			_atomNames.insert(comp);
		}
	}
}

