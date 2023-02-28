// vagabond
// Copyright (C) 2019 Helen Ginn
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

#include "GeometryTable.h"
#include "AtomGroup.h"
#include "Knotter.h"
#include <vagabond/utils/FileReader.h>
#include <vagabond/utils/version.h>
#include <iostream>

GeometryTable::GeometryTable()
{

}

void GeometryTable::addGeometryLength(std::string code, std::string pName,
                                      std::string qName, double mean, 
                                      double stdev, bool link)
{
#ifndef VERSION_PROLINE
	if (code == "PRO" && ((pName == "CD" && qName == "N") ||
	                      (qName == "CD" && pName == "N")))
	{
		return;
	}
#endif

	GeometryMap &map = _codes[code];
	
	Value value = {mean, stdev};
	AtomPair pair = {pName, qName};
	
	if (link)
	{
		_links[code].lengths[pair] = value;
		return;
	}

	map.lengths[pair] = value;
	_codes["."].lengths[pair] = value;

	pair = {qName, pName};
	map.lengths[pair] = value;
	_codes["."].lengths[pair] = value;
}

void GeometryTable::addGeometryAngle(std::string code, std::string pName, 
                                     std::string qName, std::string rName, 
                                     double mean, double stdev, bool link)
{
	GeometryMap &map = _codes[code];
	
	Value value = {mean, stdev};
	AtomTriplet trio = {pName, qName, rName};
	
	if (link)
	{
		_links[code].angles[trio] = value;
	}
	else
	{
		map.angles[trio] = value;
		_codes["."].angles[trio] = value;
	}

	trio = {rName, qName, pName};

	if (link)
	{
		_links[code].angles[trio] = value;
	}
	else
	{
		map.angles[trio] = value;
		_codes["."].angles[trio] = value;
	}
}

void GeometryTable::addGeometryTorsion(std::string code, std::string pName, 
                                       std::string qName, std::string rName, 
                                       std::string sName, double mean, 
                                       double stdev, int period)
{
	GeometryMap &map = _codes[code];
	
	Value value = {mean, stdev};
	AtomQuartet quartet = {pName, qName, rName, sName};
	map.torsions[quartet] = value;

	quartet = {sName, rName, qName, pName};
	map.torsions[quartet] = value;
}

void GeometryTable::addGeometryChiral(std::string code, std::string centre,
                                      std::string pName, std::string qName, 
                                      std::string rName, int sign)
{
#ifndef VERSION_PROLINE
	if (code == "PRO" && centre == "N")
	{
		std::cout << "giving up on proline" << std::endl;
		return;
	}
#endif

	GeometryMap &map = _codes[code];
	
	/* in the same direction as described */
	AtomQuartet quartet = {centre, pName, qName, rName};
	map.chirals[quartet] = sign;

	quartet = {centre, rName, pName, qName};
	map.chirals[quartet] = sign;

	quartet = {centre, qName, rName, pName};
	map.chirals[quartet] = sign;

	/* reverse signs in case the lookup event comes in reverse */
	quartet = {centre, qName, pName, rName};
	map.chirals[quartet] = -sign;

	quartet = {centre, pName, rName, qName};
	map.chirals[quartet] = -sign;

	quartet = {centre, rName, qName, pName};
	map.chirals[quartet] = -sign;
}


bool GeometryTable::lengthExists(std::string code, std::string pName,
                                 std::string qName)
{
	GeometryMap &map = _codes[code];

	AtomPair pair = {pName, qName};
	return (map.lengths.count(pair) > 0);
}

double GeometryTable::length(const GeometryMap &map, std::string pName, 
                             std::string qName) const
{
	AtomPair pair = {pName, qName};
	if (map.lengths.count(pair) == 0)
	{
		return -1;
	}

	const Value &v = map.lengths.at(pair);

	return v.mean;
}

double GeometryTable::length(std::string code, std::string pName,
                             std::string qName, bool links) const
{
	if (_codes.count(code))
	{
		const GeometryMap &map = _codes.at(code);

		if (!links)
		{
			double l = length(map, pName, qName);

			if (l >= 0)
			{
				return l;
			}
		}
		else
		{
			double l = checkLengthLinks(code, pName, qName);
			return l;
		}
	}

	if (_codes.count(".") == 0)
	{
		return -1;
	}

	double l = -1;
	if (_filterPeptides && 
	    (!(pName == "N" && qName == "C") && 
	     !(pName == "C" && qName == "N")))
	{
		l = length(_codes.at("."), pName, qName);
	}
	return l;
}

double GeometryTable::length_stdev(std::string code, std::string pName,
                                   std::string qName)
{
	GeometryMap &map = _codes[code];

	AtomPair pair = {pName, qName};
	Value &v = map.lengths[pair];

	return v.stdev;
}

bool GeometryTable::angleExists(std::string code, std::string pName,
                                std::string qName, std::string rName)
{
	return angle(code, pName, qName, rName) >= 0;
}

double GeometryTable::angle(GeometryMap &map, std::string pName,
                            std::string qName, std::string rName)
{
	AtomTriplet trio = {pName, qName, rName};
	if (map.angles.count(trio) == 0)
	{
		return -1;
	}

	Value &v = map.angles[trio];

	return v.mean;
}

double GeometryTable::angle(std::string code, std::string pName,
                            std::string qName, std::string rName,
                            bool links)
{
	AtomTriplet trio = {pName, qName, rName};

	if (!links)
	{
		double a = angle(_codes[code], pName, qName, rName);

		if (a < 0)
		{
			a = angle(_codes["."], pName, qName, rName);
		}
		
		return a;
	}
	else
	{
		return checkAngleLinks(code, pName, qName, rName);
	}
}

double GeometryTable::angle_stdev(std::string code, std::string pName,
                                  std::string qName, std::string rName)
{
	GeometryMap &map = _codes[code];

	AtomTriplet trio = {pName, qName, rName};
	if (map.angles.count(trio) == 0)
	{
		return -1;
	}

	Value &v = map.angles[trio];

	return v.stdev;
}


bool GeometryTable::torsionExists(std::string code, std::string pName,
                                  std::string qName, std::string rName,
                                  std::string sName)
{
	GeometryMap &map = _codes[code];
	
	AtomQuartet quartet = {pName, qName, rName, sName};
	return (map.torsions.count(quartet) > 0);
}

double GeometryTable::torsion(std::string code, std::string pName,
                              std::string qName, std::string rName,
                              std::string sName)
{
	GeometryMap &map = _codes[code];

	AtomQuartet quartet = {pName, qName, rName, sName};
	if (map.torsions.count(quartet) == 0)
	{
		return -1;
	}

	Value &v = map.torsions[quartet];

	return v.mean;
}

double GeometryTable::torsion_stdev(std::string code, std::string pName,
                                    std::string qName, std::string rName,
                                    std::string sName)
{
	GeometryMap &map = _codes[code];

	AtomQuartet quartet = {pName, qName, rName, sName};
	if (map.torsions.count(quartet) == 0)
	{
		return -1;
	}

	Value &v = map.torsions[quartet];

	return v.stdev;
}

int GeometryTable::chirality(std::string code, std::string centre,
                             std::string pName, std::string qName, 
                             std::string rName)
{
	GeometryMap &map = _codes[code];

	AtomQuartet quartet = {centre, pName, qName, rName};
	if (map.chirals.count(quartet) == 0)
	{
		return 0;
	}

	int &v = map.chirals[quartet];

	return v;
}

const size_t GeometryTable::codeEntries() const
{
	return _codes.size();
}

bool GeometryTable::linkCodeMatches(std::string code, std::string query) const
{
	if (query == ".")
	{
		return true;
	}
	else if (query.rfind("NOT", 0) != std::string::npos)
	{
		std::vector<std::string> opts = split(query, ' ');

		bool ok = true;
		for (size_t i = 1; i < opts.size(); i++)
		{
			if (code == opts[i])
			{
				ok = false;
				break;
			}
		}

		if (ok)
		{
			return ok;
		}
	}
	else /* does not start with NOT */
	{
		std::vector<std::string> opts = split(query, ' ');

		for (size_t i = 0; i < opts.size(); i++)
		{
			if (code == opts[i])
			{
				return true;
			}
		}
	}

	return false;
}

double GeometryTable::checkAngleLinks(std::string code, std::string pName,
                                      std::string qName, std::string rName)
{
	std::map<std::string, GeometryMap>::iterator it;
	
	for (it = _links.begin(); it != _links.end(); it++)
	{
		std::string q = it->first;
		bool match = linkCodeMatches(code, q);
		
		if (!match)
		{
			continue;
		}
		
		GeometryMap &map = it->second;
		double a = angle(map, pName, qName, rName);

		if (a >= 0)
		{
			return a;
		}
	}

	return -1;
}

double GeometryTable::checkLengthLinks(std::string code, std::string pName,
                                      std::string qName) const
{
	std::map<std::string, GeometryMap>::const_iterator it;
	
	for (it = _links.cbegin(); it != _links.cend(); it++)
	{
		std::string q = it->first;
		bool match = linkCodeMatches(code, q);
		
		if (!match)
		{
			continue;
		}
		
		const GeometryMap &map = it->second;
		double l = length(map, pName, qName);

		if (l >= 0)
		{
			return l;
		}
	}

	return -1;
}

std::set<std::string> GeometryTable::allAtomNames(std::string &code)
{
	std::set<std::string> atoms;
	
	const GeometryMap &map = _codes[code];
	
	for (auto it = map.lengths.begin(); it != map.lengths.end(); it++)
	{
		const AtomPair &ap = it->first;
		atoms.insert(ap.p);
		atoms.insert(ap.q);
	}
	
	return atoms;
}

AtomGroup *GeometryTable::constructResidue(std::string code, 
                                           const ResidueId &id, int *num,
                                           int terminal)
{
	AtomGroup *ag = new AtomGroup();
	std::set<std::string> names = allAtomNames(code);
	
	for (const std::string &name : names)
	{
		if ((terminal < 1 && name == "OXT"))
		{
			continue;
		}

		std::string ele = std::string();
		ele += name[0];
		// hacky, relying on the fact CNHOSP all have a single letter.
		// not great.
		
		if (ele == "H")
		{
			continue;
		}

		Atom *a = new Atom();
		a->setElementSymbol(ele);
		a->setAtomName(name);
		a->setAtomNum(*num);
		a->setResidueId(id);
		a->setInitialPosition(glm::vec3(0.f), 30);
		a->setChain("A");
		a->setCode(code);
		
		*ag += a;
		(*num)++;
	}
	
	Knotter knot(ag, this);
	knot.setDoAngles(false);
	knot.setDoTorsions(false);
	knot.knot();
	
	return ag;
}
