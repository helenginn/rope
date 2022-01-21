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

#include <string>
#include <map>

class GeometryTable
{
public:
	GeometryTable();

	void addGeometryLength(std::string code, std::string pName,
	                       std::string qName, double mean, double stdev);

	void addGeometryAngle(std::string code, std::string pName, 
	                      std::string qName, std::string rName, 
	                      double mean, double stdev);

	void addGeometryTorsion(std::string code, std::string pName, 
	                        std::string qName, std::string rName, 
	                        std::string sName, double mean, 
	                        double stdev, int period);

	bool lengthExists(std::string code, std::string pName, std::string qName);

	double length(std::string code, std::string pName, std::string qName);

	double length_stdev(std::string code, std::string pName, std::string qName);

	bool angleExists(std::string code, std::string pName, std::string qName,
	                 std::string rName);
	double angle(std::string code, std::string pName, std::string qName,
	                 std::string rName);
	double angle_stdev(std::string code, std::string pName, std::string qName,
	                 std::string rName);

	bool torsionExists(std::string code, std::string pName, std::string qName,
	                   std::string rName, std::string sName);
	double torsion(std::string code, std::string pName, std::string qName,
	               std::string rName, std::string sName);
	double torsion_stdev(std::string code, std::string pName, std::string qName,
	                     std::string rName, std::string sName);
private:

};

#endif
