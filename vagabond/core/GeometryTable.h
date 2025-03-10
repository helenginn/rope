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

#ifndef __vagabond__GeometryTable__
#define __vagabond__GeometryTable__

#include <vector>
#include <string>
#include <mutex>
#include <set>
#include <map>
#include <iostream>

#include "BackboneType.h"

struct ResidueId;
class AtomGroup;

class GeometryTable
{
public:
	GeometryTable();
	
	static GeometryTable &table()
	{
		return _loadedGeometry;
	}
	
	static GeometryTable &getAllGeometry();
	
	static void loadExtraGeometries(const std::set<std::string> &geometries);
	
	void setBackboneType(const std::string &code, const BackboneType &type)
	{
		_backboneTypes[code] = type;
	}
	
	BackboneType backboneType(const std::string &code)
	{
		if (_backboneTypes.count(code))
		{
			return _backboneTypes.at(code);
		}
		
		return NoOverride;
	}

	void addGeometryLength(std::string code, std::string pName,
	                       std::string qName, double mean, double stdev, 
	                       bool link = false);

	void addGeometryAngle(std::string code, std::string pName, 
	                      std::string qName, std::string rName, 
	                      double mean, double stdev, bool link = false);

	void addGeometryTorsion(std::string code, std::string pName, 
	                        std::string qName, std::string rName, 
	                        std::string sName, double mean, 
	                        double stdev, int period);

	void addGeometryChiral(std::string code, std::string centre,
	                        std::string pName, std::string qName, 
	                       std::string rName, int sign);

	bool lengthExists(std::string code, std::string pName, std::string qName);

	std::pair<double, double> length(std::string code, std::string pName, 
	                               std::string qName, 
	                               bool links = false);

	double length_stdev(std::string code, std::string pName, std::string qName);

	bool angleExists(std::string code, std::string pName, std::string qName,
	                 std::string rName);
	double angle(std::string code, std::string pName, std::string qName,
	                 std::string rName, bool links = false) const;
	double angle_stdev(std::string code, std::string pName, std::string qName,
	                 std::string rName, bool links = false) const;

	bool torsionExists(std::string code, std::string pName, std::string qName,
	                   std::string rName, std::string sName);
	double torsion(std::string code, std::string pName, std::string qName,
	               std::string rName, std::string sName);
	double torsion_stdev(std::string code, std::string pName, std::string qName,
	                     std::string rName, std::string sName);

	int chirality(std::string code, std::string centre, std::string pName,
	              std::string qName, std::string rName);

	void filterPeptides(bool filter)
	{
		_filterPeptides = filter;
	}
	
	const size_t codeEntries() const;
	
	AtomGroup *constructResidue(std::string code, const ResidueId &id, 
	                            int *atomNum, int terminal = 0);

private:
	void loadExtraGeometryFiles(std::set<std::string> &files);

	struct Value
	{
		double mean;
		double stdev;
	};
	struct AtomPair
	{
		std::string p;
		std::string q;
		
		bool operator<(const AtomPair &b) const
		{
			if (this->p == b.p)
			{
				return this->q < b.q;
			}
			else
			{
				return this->p < b.p;
			}
		}
	};

	struct AtomTriplet
	{
		std::string p;
		std::string q;
		std::string r;
		
		bool operator<(const AtomTriplet &b) const
		{
			if (this->p == b.p)
			{
				if (this->q == b.q)
				{
					return this->r < b.r;
				}
				else
				{
					return this->q < b.q;
				}
			}
			else
			{
				return this->p < b.p;
			}
		}
	};

	struct AtomQuartet
	{
		std::string p;
		std::string q;
		std::string r;
		std::string s;
		
		bool operator<(const AtomQuartet &b) const
		{
			if (this->p == b.p)
			{
				if (this->q == b.q)
				{
					if (this->r == b.r)
					{
						return this->s < b.s;
					}
					else
					{
						return this->r < b.r;
					}
				}
				else
				{
					return this->q < b.q;
				}
			}
			else
			{
				return this->p < b.p;
			}
		}
	};

	struct GeometryMap
	{
		std::map<AtomPair, Value> lengths;
		std::map<AtomTriplet, Value> angles;
		std::map<AtomQuartet, Value> torsions;
		std::map<AtomQuartet, int> chirals;
	};

	Value angle_value(std::string code, std::string pName, std::string qName,
	                 std::string rName, bool links = false) const;
	Value angle_value(const GeometryMap &map, std::string pName,
	                  std::string qName, std::string rName) const;
	Value angle_value(const GeometryMap &map, std::string pName,
	                  std::string qName, std::string rName, bool links) const;
	double angle_stdev(const GeometryMap &map, std::string pName,
	                   std::string qName, std::string rName) const;

	std::set<std::string> allAtomNames(std::string &code);

	std::pair<double, double> length(const GeometryMap &map, std::string pName, 
	                                 std::string qName) const;

	std::pair<double, double> checkLengthLinks(std::string code,
	                                           std::string pName,
	                                           std::string qName) const;

	Value checkAngleLinks(std::string code, std::string pName,
	                      std::string qName, std::string rName) const;

	bool linkCodeMatches(std::string code, std::string query) const;
	double angle(const GeometryMap &map, std::string pName,
	             std::string qName, std::string rName) const;
	
	std::map<std::string, GeometryMap> _codes;
	std::map<std::string, GeometryMap> _links;
	std::map<std::string, BackboneType> _backboneTypes;
	
	static GeometryTable _loadedGeometry;
	std::set<std::string> _loadedFiles;
	std::mutex *_mutex = nullptr;

	bool _filterPeptides = false;
};

#endif
