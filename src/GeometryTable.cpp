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

GeometryTable::GeometryTable()
{

}

void GeometryTable::addGeometryLength(std::string code, std::string pName,
                                      std::string qName, double mean, 
                                      double stdev)
{

}

void GeometryTable::addGeometryAngle(std::string code, std::string pName, 
                                     std::string qName, std::string rName, 
                                     double mean, double stdev)
{

}

void GeometryTable::addGeometryTorsion(std::string code, std::string pName, 
                                       std::string qName, std::string rName, 
                                       std::string sName, double mean, 
                                       double stdev, int period)
{

}

bool GeometryTable::lengthExists(std::string code, std::string pName,
                                 std::string qName)
{
	return false;
}

double GeometryTable::length(std::string code, std::string pName,
                             std::string qName)
{
	return 0;
}

double GeometryTable::length_stdev(std::string code, std::string pName,
                                   std::string qName)
{
	return 0;
}

bool GeometryTable::angleExists(std::string code, std::string pName,
                                std::string qName, std::string rName)
{
	return false;
}

double GeometryTable::angle(std::string code, std::string pName,
                            std::string qName, std::string rName)
{
	return 0;
}

double GeometryTable::angle_stdev(std::string code, std::string pName,
                                  std::string qName, std::string rName)
{
	return 0;
}


bool GeometryTable::torsionExists(std::string code, std::string pName,
                                  std::string qName, std::string rName,
                                  std::string sName)
{
	return false;
}

double GeometryTable::torsion(std::string code, std::string pName,
                              std::string qName, std::string rName,
                              std::string sName)
{
	return 0;
}

double GeometryTable::torsion_stdev(std::string code, std::string pName,
                                    std::string qName, std::string rName,
                                    std::string sName)
{
	return 0;
}

