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
#include "files/CifFile.h"
#include "Knotter.h"
#include "Environment.h"
#include "FileManager.h"
#include "BondLength.h"
#include <vagabond/utils/FileReader.h>
#include <vagabond/utils/version.h>
#include <iostream>

GeometryTable GeometryTable::_loadedGeometry;

GeometryTable::GeometryTable()
{
	_mutex = new std::mutex();

}

void GeometryTable::loadExtraGeometryFiles(std::set<std::string> &files)
{
	std::unique_lock<std::mutex> lock(*_mutex);
	
	for (const std::string &file : files)
	{
		if (_loadedFiles.count(file))
		{
			continue;
		}

		CifFile cf(file);
		cf.setGeometryTable(this);
		cf.parse();
		_loadedFiles.insert(file);
	}
}


GeometryTable &GeometryTable::getAllGeometry()
{
	FileManager *fm = Environment::fileManager();
	std::set<std::string> files = fm->geometryFiles();
	files.insert("assets/geometry/standard_geometry.cif");

	_loadedGeometry.loadExtraGeometries(files);
	return _loadedGeometry;
}

void GeometryTable::loadExtraGeometries(const std::set<std::string> &geometries)
{
	std::set<std::string> files = geometries;
	files.insert("assets/geometry/standard_geometry.cif");
	_loadedGeometry.loadExtraGeometryFiles(files);

}

void GeometryTable::addGeometryLength(std::string code, std::string pName,
                                      std::string qName, double mean, 
                                      double stdev, bool link)
{
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

std::pair<double, double> GeometryTable::length(const GeometryMap &map,
                                                std::string pName, 
                                                std::string qName) const
{
	AtomPair pair = {pName, qName};
	if (map.lengths.count(pair) == 0)
	{
		return {-1, 0};
	}

	const Value &v = map.lengths.at(pair);

	return {v.mean, v.stdev};
}

std::pair<double, double> GeometryTable::length(std::string code,
                                                std::string pName,
                                                std::string qName, bool links) 
{
	if (_codes.count(code))
	{
		const GeometryMap &map = _codes.at(code);

		if (!links)
		{
			auto l = length(map, pName, qName);

			if (l.first >= 0)
			{
				return l;
			}
		}
		else
		{
			auto l = checkLengthLinks(code, pName, qName);
			return l;
		}
	}

	if (_codes.count(".") == 0 || !links)
	{
		return {-1, 0};
	}

	if (_filterPeptides && 
	    (!(pName == "N" && qName == "C") && 
	     !(pName == "C" && qName == "N")))
	{
		auto l = length(_codes.at("."), pName, qName);
		return l;
	}
	return {-1, 0};
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

GeometryTable::Value GeometryTable::angle_value(const GeometryMap &map, std::string pName,
                                           std::string qName, std::string rName) const
{
	AtomTriplet trio = {pName, qName, rName};
	if (map.angles.count(trio) == 0)
	{
		return {-1, -1};
	}

	const Value &v = map.angles.at(trio);

	return v;
}

double GeometryTable::angle(const GeometryMap &map, std::string pName,
                            std::string qName, std::string rName) const
{
	return angle_value(map, pName, qName, rName).mean;
}

double GeometryTable::angle_stdev(const GeometryMap &map, std::string pName,
                                  std::string qName, std::string rName) const
{
	return angle_value(map, pName, qName, rName).stdev;
}

GeometryTable::Value GeometryTable::angle_value(std::string code, std::string pName,
                  std::string qName, std::string rName, bool links) const
{
	AtomTriplet trio = {pName, qName, rName};
	

	if (!links)
	{
		Value a{-1, -1};;
		if (_codes.count(code))
		{
			a = angle_value(_codes.at(code), pName, qName, rName);

			if (a.mean < 0)
			{
				a = angle_value(_codes.at("."), pName, qName, rName);
			}
		}
		else
		{
			a = angle_value(_codes.at("."), pName, qName, rName);
		}

		return a;
	}
	else
	{
		return checkAngleLinks(code, pName, qName, rName);
	}
}

double GeometryTable::angle(std::string code, std::string pName,
                            std::string qName, std::string rName, bool links) const
{
	return angle_value(code, pName, qName, rName, links).mean;
}

double GeometryTable::angle_stdev(std::string code, std::string pName,
                                  std::string qName, std::string rName, bool links) const
{
	return angle_value(code, pName, qName, rName, links).stdev;
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
	for (auto it = map.chirals.begin(); it != map.chirals.end(); it++)
	{
		const AtomQuartet &quartet = it->first;
	}

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

GeometryTable::Value GeometryTable::checkAngleLinks(std::string code,
                                               std::string pName,
                                               std::string qName,
                                               std::string rName) const
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
		Value v = angle_value(map, pName, qName, rName);

		if (v.mean >= 0)
		{
			return v;
		}
	}

	return {-1, -1};
}

std::pair<double, double> 
GeometryTable::checkLengthLinks(std::string code, std::string pName,
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
		auto l = length(map, pName, qName);

		if (l.first >= 0)
		{
			return l;
		}
	}

	return {-1, 0};
}

std::set<std::string> GeometryTable::allAtomNames(std::string &code)
{
	std::set<std::string> atoms;
	
	if (_codes.count(code) == 0)
	{
		throw std::runtime_error("Could not find code " + code + 
		                         " in geometry tables.");
	}
	
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

GeometryTable *File::geometryTable()
{
	if (!_table)
	{
		_table = new GeometryTable();
	}

	return _table;
}

