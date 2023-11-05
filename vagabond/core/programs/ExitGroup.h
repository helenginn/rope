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

class Atom;
#include <nlohmann/json.hpp>
#include "Atom.h"
#include <sstream>
using nlohmann::json;

namespace rope
{
struct ExitGroup
{
public:
	struct Flaggable
	{
		std::string name;
		int idx;
		bool central;
		bool belongs;
		bool entry;
		Atom *ptr;
	};
	
	/* exit group is not required to complete if an atom entry exists but
	 * the found central atom doesn't have the name in its neighbours */
	bool required()
	{
		// must find the central atom before judging requirements
		if (!central()->ptr) 
		{
			return true;
		}

		std::string entry_name = entry()->name;
		Atom *centre = central()->ptr;
		
		for (size_t i = 0; i < centre->bondLengthCount(); i++)
		{
			Atom *neighbour = centre->connectedAtom(i);
			std::string n = neighbour->atomName();
			
			// if the atom exists, we must wait for it.
			if (n == entry_name)
			{
				return true;
			}
		}

		return false;
	}

	Flaggable *entry()
	{
		for (Flaggable &f : atoms)
		{
			if (f.entry)
			{
				return &f;
			}
		}

		return nullptr;

	}

	Flaggable *central()
	{
		for (Flaggable &f : atoms)
		{
			if (f.central)
			{
				return &f;
			}
		}

		return nullptr;
	}

	bool hasAtom(Atom *ptr)
	{
		for (Flaggable &f : atoms)
		{
			if (f.ptr == ptr)
			{
				return true;
			}
		}

		return false;
	}

	void addCentral(std::string name)
	{
		atoms.push_back(Flaggable{name, -1, true, false, false, nullptr});
	}

	void add(std::string name, bool entry = false)
	{
		atoms.push_back(Flaggable{name, -1, false, false, entry, nullptr});
	}

	bool allFlagged()
	{
		for (Flaggable &f : atoms)
		{
			if (f.idx < 0)
			{
				return false;
			}
		}

		return true;
	}

	std::vector<Flaggable> atoms;
	bool chosen_entry = false;
	bool skip = false;
	
	friend std::ostream &operator<<(std::ostream &ss, const ExitGroup &grp) 
	{
		ss << "{";
		for (const Flaggable &f : grp.atoms)
		{
			ss << "(" << f.name << " = ";
			ss << (f.ptr ? f.ptr->desc() : "(null)");
			ss << ", ";
			ss << f.idx;
			
			if (f.central) ss << ", central";
			if (f.entry) ss << ", entry";
			ss << "),";
		}
		ss << "}";
		return ss;
	}
};

inline void to_json(json &j, const ExitGroup::Flaggable &value)
{
	j["name"] = value.name;
	j["central"] = value.central;
	j["entry"] = value.entry;
	j["belongs"] = value.belongs;
}

inline void from_json(const json &j, ExitGroup::Flaggable &value)
{
	value.name = j.at("name");
	value.central = j.at("central");
	value.entry = j.at("entry");
	
	if (j.count("belongs"))
	{
		value.belongs = j.at("belongs");
	}

	value.idx = -1;
	value.ptr = nullptr;
}

inline void to_json(json &j, const ExitGroup &value)
{
	j["atoms"] = value.atoms;
	j["skip"] = value.skip;
}

inline void from_json(const json &j, ExitGroup &value)
{
	std::vector<ExitGroup::Flaggable> atoms = j.at("atoms");
	value.atoms = atoms;

	if (j.count("skip"))
	{
		value.skip = j.at("skip");
	}
}

};

