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

#ifndef __vagabond__DataGroup__
#define __vagabond__DataGroup__

#include <vector>
#include <string>

template <class Unit>
class DataGroup
{
public:
	DataGroup(int length);
	virtual ~DataGroup() {};

	typedef std::vector<Unit> Array;

	void makeAverage();
	void findDifferences();
	void write(std::string filename);
	virtual void addArray(std::string name, Array next);
	
	void addHeader(std::string header);
	void addUnitNames(std::vector<std::string> header);
protected:
	std::vector<Array> _vectors;
	std::vector<Array> _diffs;
	std::vector<std::string> _unitNames;
	std::vector<std::string> _vectorNames;

	int _length;
	Array _average;
};

#include "DataGroup.cpp"

#endif
