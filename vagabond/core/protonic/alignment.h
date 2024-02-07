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

#ifndef __vagabond__alignment__
#define __vagabond__alignment__

#include "hnet.h"
#include "Connector.h"
#include "Superpose.h"
#include <vagabond/utils/OpSet.h>

class Atom;

namespace hnet
{

typedef std::pair<::Atom *, hnet::BondConnector *> ABPair;
typedef OpSet<ABPair> PairSet;

inline std::vector<std::string> inactiveHydrogenNames(::Atom *atom)
{
	struct NameInfo
	{
		std::string code;
		std::string name;
		std::vector<std::string> hydroNames;
	};

	std::vector<NameInfo> list = {{"GLY", "CA", {"HA1", "HA2"}},
                           	      {"", "CA", {"HA"}},
	                              {"ALA", "CB", {"HB1", "HB2", "HB3"}},
	                              {"ARG", "CB", {"HB2", "HB3"}},
	                              {"ARG", "CG", {"HG2", "HG3"}},
	                              {"ARG", "CD", {"HD2", "HD3"}},
	                              {"ASN", "CB", {"HB2", "HB3"}},
	                              {"ASP", "CB", {"HB2", "HB3"}},
	                              {"CYS", "CB", {"HB2", "HB3"}},
	                              {"GLN", "CB", {"HB2", "HB3"}},
	                              {"GLN", "CG", {"HG2", "HG3"}},
	                              {"GLU", "CB", {"HB2", "HB3"}},
	                              {"GLU", "CG", {"HG2", "HG3"}},
	                              {"HIS", "CB", {"HB2", "HB3"}},
	                              {"HIS", "CD2", {"HD2"}},
	                              {"HIS", "CE1", {"HE1"}},
	                              {"SER", "CB", {"HB2", "HB3"}},
	                              {"ILE", "CB", {"HB"}},
	                              {"ILE", "CG1", {"HG12", "HG13"}},
	                              {"ILE", "CG2", {"HG12", "HG22", "HG23"}},
	                              {"ILE", "CD1", {"HD11", "HD12", "HD13"}},
	                              {"LEU", "CB", {"HB2", "HB3"}},
	                              {"LEU", "CG", {"HG"}},
	                              {"LEU", "CD1", {"HD11", "HD12", "HD13"}},
	                              {"LEU", "CD2", {"HD21", "HD22", "HD23"}},
	                              {"LYS", "CB", {"HB2", "HB3"}},
	                              {"LYS", "CG", {"HG2", "HG3"}},
	                              {"LYS", "CD", {"HD2", "HD3"}},
	                              {"LYS", "CE", {"HE2", "HE3"}},
	                              {"MET", "CB", {"HB2", "HB3"}},
	                              {"MET", "CG", {"HG2", "HG3"}},
	                              {"MET", "CE", {"HE1", "HE2", "HE3"}},
	                              {"PHE", "CB", {"HB2", "HB3"}},
	                              {"PHE", "CD1", {"HD1"}},
	                              {"PHE", "CD2", {"HD2"}},
	                              {"PHE", "CE1", {"HE1"}},
	                              {"PHE", "CE2", {"HE2"}},
	                              {"PHE", "CZ", {"HZ"}},
	                              {"PRO", "CB", {"HB2", "HB3"}},
	                              {"PRO", "CG", {"HG2", "HG3"}},
	                              {"PRO", "CD", {"HD2", "HD3"}},
	                              {"THR", "CG2", {"HG12", "HG22", "HG23"}},
	                              {"THR", "CB", {"HB"}},
	                              {"TRP", "CB", {"HB2", "HB3"}},
	                              {"TRP", "CD1", {"HD1"}},
	                              {"TRP", "CE3", {"HE3"}},
	                              {"TRP", "CZ3", {"HZ3"}},
	                              {"TRP", "CH2", {"HH2"}},
	                              {"TRP", "CZ2", {"HZ2"}},
	                              {"TYR", "CB", {"HB2", "HB3"}},
	                              {"TYR", "CD1", {"HD1"}},
	                              {"TYR", "CD2", {"HD2"}},
	                              {"TYR", "CE1", {"HE1"}},
	                              {"TYR", "CE2", {"HE2"}},
	                              {"VAL", "CG1", {"HG11", "HG12", "HG13"}},
	                              {"VAL", "CG2", {"HG21", "HG22", "HG23"}},
	};

	for (NameInfo &info : list)
	{
		if ((atom->code() != info.code && info.code.length() > 0)
		    || atom->atomName() != info.name)
		{
			continue;
		}
		
		return info.hydroNames;
	}
	
	return {};

}

inline OpSet<::Atom *> inactiveHydrogens(::Atom *atom, int &coord)
{
	struct InactiveInfo
	{
		std::string code;
		std::string name;
		std::set<std::string> neighbours;
		int coord_state; // when over 10, check second layer of contacts!
	};

	std::vector<InactiveInfo> list = {{"GLY", "CA", {"N", "C"}, 4},
	                                  {"", "CA", {"N", "C", "CB"}, 4},
	                                  {"ALA", "CB", {"N", "CA"}, 14},
	                                  {"ARG", "CB", {"CA", "CG"}, 4},
	                                  {"ARG", "CG", {"CB", "CD"}, 4},
	                                  {"ARG", "CD", {"CG", "NE"}, 4},
	                                  {"ASN", "CB", {"CA", "CG"}, 4},
	                                  {"ASP", "CB", {"CA", "CG"}, 4},
	                                  {"CYS", "CB", {"CA", "SG"}, 4},
	                                  {"GLN", "CB", {"CA", "CG"}, 4},
	                                  {"GLN", "CG", {"CB", "CD"}, 4},
	                                  {"GLU", "CB", {"CA", "CG"}, 4},
	                                  {"GLU", "CG", {"CB", "CD"}, 4},
	                                  {"HIS", "CB", {"CA", "CG"}, 4},
	                                  {"HIS", "CD2", {"NE2", "CG"}, 3},
	                                  {"HIS", "CE1", {"NE2", "ND1"}, 3},
	                                  {"ILE", "CB", {"CA", "CG1", "CG2"}, 4},
	                                  {"ILE", "CG1", {"CB", "CD1"}, 4},
	                                  {"ILE", "CG2", {"CA", "CB"}, 14},
	                                  {"ILE", "CD1", {"CB", "CG1"}, 14},
	                                  {"LEU", "CB", {"CA", "CG"}, 4},
	                                  {"LEU", "CG", {"CB", "CD1", "CD2"}, 4},
	                                  {"LEU", "CD1", {"CB", "CG"}, 14},
	                                  {"LEU", "CD2", {"CB", "CG"}, 14},
	                                  {"LYS", "CB", {"CA", "CG"}, 4},
	                                  {"LYS", "CG", {"CB", "CD"}, 4},
	                                  {"LYS", "CD", {"CG", "CE"}, 4},
	                                  {"LYS", "CE", {"CD", "NZ"}, 4},
	                                  {"MET", "CB", {"CA", "CG"}, 4},
	                                  {"MET", "CG", {"CB", "SD"}, 4},
	                                  {"MET", "CE", {"CG", "SD"}, 14},
	                                  {"PHE", "CB", {"CA", "CG"}, 4},
	                                  {"PHE", "CD1", {"CG", "CE1"}, 3},
	                                  {"PHE", "CD2", {"CG", "CE2"}, 3},
	                                  {"PHE", "CE1", {"CZ", "CD1"}, 3},
	                                  {"PHE", "CE2", {"CZ", "CD2"}, 3},
	                                  {"PHE", "CZ", {"CE1", "CE2"}, 3},
	                                  {"PRO", "CB", {"CA", "CG"}, 4},
	                                  {"PRO", "CG", {"CB", "CD"}, 4},
	                                  {"PRO", "CD", {"N", "CG"}, 4},
	                                  {"PRO", "CD", {"N", "CG"}, 4},
	                                  {"SER", "CB", {"CA", "OG"}, 4},
	                                  {"THR", "CB", {"CA", "OG1", "CG2"}, 4},
	                                  {"THR", "CG2", {"CA", "CB"}, 14},
	                                  {"TRP", "CB", {"CA", "CG"}, 4},
	                                  {"TRP", "CD1", {"NE1", "CG"}, 3},
	                                  {"TRP", "CE3", {"CD2", "CZ3"}, 3},
	                                  {"TRP", "CZ3", {"CE3", "CH2"}, 3},
	                                  {"TRP", "CH2", {"CZ3", "CZ2"}, 3},
	                                  {"TRP", "CZ2", {"CH2", "CE2"}, 3},
	                                  {"TYR", "CB", {"CA", "CG"}, 4},
	                                  {"TYR", "CD1", {"CG", "CE1"}, 3},
	                                  {"TYR", "CD2", {"CG", "CE2"}, 3},
	                                  {"TYR", "CE1", {"CZ", "CD1"}, 3},
	                                  {"TYR", "CE2", {"CZ", "CD2"}, 3},
	                                  {"VAL", "CG1", {"CA", "CB"}, 14},
	                                  {"VAL", "CG2", {"CA", "CB"}, 14},
									  };

	OpSet<::Atom *> found;
	for (InactiveInfo &info : list)
	{
		if ((atom->code() != info.code && info.code.length() > 0)
		    || atom->atomName() != info.name)
		{
			continue;
		}
		
		std::cout << "Checking " << atom->desc() << ": ";
		
		auto add_matching_to_found = [&found, &info](::Atom *atom)
		{
			for (size_t i = 0; i < atom->bondLengthCount(); i++)
			{
				std::string other = atom->connectedAtom(i)->atomName();
				if (info.neighbours.count(other))
				{
					std::cout << atom->connectedAtom(i)->desc() << " ";
					found.insert(atom->connectedAtom(i));
				}
			}
		};

		if (info.coord_state > 10)
		{
			for (size_t i = 0; i < atom->bondLengthCount(); i++)
			{
				add_matching_to_found(atom->connectedAtom(i));
			}
		}
		
		add_matching_to_found(atom);

		std::cout << std::endl;

		coord = info.coord_state;
		break;
	}
	
	return found;
}

/* finding the "uninvolved" atom for throwing out non-bondable possibilities
 * due to violating bond angle: Ser-CB---O:::H---X where CB-O-X must be reasonable
 * values */
inline OpSet<ABPair> uninvolvedCoordinators(::Atom *atom)
{
	struct FindUninvolved
	{
		std::string code;
		std::string active;
		std::string uninvolved;
	};

	std::vector<FindUninvolved> list = {{"SER", "OG", "CB"},
		                                {"THR", "OG1", "CB"},
		                                {"TYR", "OH", "CZ"},
		                                {"", "N", "CA"},
		                                {"", "N", "C"},
		                                {"", "O", "C"},
		                                {"GLN", "OE1", "CD"},
		                                {"GLN", "NE2", "CD"},
		                                {"ASN", "OD1", "CG"},
		                                {"ASN", "ND2", "CG"},
		                                {"ASP", "OD1", "CG"},
		                                {"ASP", "OD2", "CG"},
		                                {"LYS", "NZ", "CE"}};

	OpSet<ABPair> found;
	for (FindUninvolved &find : list)
	{
		if ((atom->code() != find.code && find.code.length() > 0)
		    || atom->atomName() != find.active)
		{
			continue;
		}
		
		for (size_t i = 0; i < atom->bondLengthCount(); i++)
		{
			if (atom->connectedAtom(i)->atomName() == find.uninvolved)
			{
				found.insert({atom->connectedAtom(i), nullptr});
			}
		}
	}
	
	return found;
}

inline float expected_angle_for_coordination(int coord)
{
	switch (coord)
	{
		case 2: return 180;
		case 3: return 120;
		case 4: return 109.5;
		default: return 0;
	}
}

inline std::vector<glm::vec3> template_for_coordination(int coord)
{
	if (coord == 14)
	{
		// for e.g. Ala-CB, first atom is where N should be
		return {{2.0, -2.0, 0},
				{1, -1, 1}, {-1, -1, -1}, {-1, 1, 1}, {1, 1, -1}};
	}
	if (coord == 4)
	{
		// tetrahedral coordination around centre (origin)
		return {{1, -1, 1}, {-1, -1, -1}, {-1, 1, 1}, {1, 1, -1}};
	}
	else if (coord == 3)
	{
		// trigonal coordination around centre (origin)
		return {{1, 0, 0}, {-0.5, 0.866, 0}, {-0.5, -0.866, 0}};
	}
	else if (coord == 2)
	{
		// trigonal coordination around centre (origin)
		return {{1, 0, 0}, {-1.0, 0, 0}};
	}
	
	return {};
}

inline std::vector<glm::vec3> align(int coordNum, const glm::vec3 &centre,
                                    const std::vector<glm::vec3> &some,
                                    float mult = 1.f)
{
	std::vector<glm::vec3> blueprint = template_for_coordination(coordNum);
	
	if (blueprint.size() == 0) return {};

	Superpose pose;
	pose.setForcedMeans({}, {});
	
	glm::vec3 blueprint_centre = {};
	pose.addPositionPair(blueprint_centre, blueprint_centre); // centre

	for (size_t i = 0; i < some.size(); i++)
	{
		glm::vec3 dir = glm::normalize(some[i] - centre);
		glm::vec3 expected = glm::normalize(blueprint[i]);
		pose.addPositionPair(dir, expected);
	}

	pose.superpose();
	glm::mat3x3 super = pose.rotation();
	
	std::vector<glm::vec3> to_return;

	for (size_t i = 0; i < coordNum; i++)
	{
		glm::vec3 pos = super * glm::normalize(blueprint[i]);
		pos *= mult;
		pos += centre;
		to_return.push_back(pos);
	}
	
	return to_return;
}
};

#endif
