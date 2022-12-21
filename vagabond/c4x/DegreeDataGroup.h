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

#ifndef __vagabond__DegreeDataGroup__
#define __vagabond__DegreeDataGroup__

#include "Angular.h"
#include "DataGroup.h"

/** \class DegreeDataGroup
 * specialised class of DataGroup that takes degree values and ensures all
 * values match previous entries as closely as possible by adding or subtracting
 * multiple of 360 degrees */


template <class Header>
class DegreeDataGroup : public DataGroup<Angular, Header>
{
public:
	using Array = typename DataGroup<Angular, Header>::Array;
	
	DegreeDataGroup(int num) : DataGroup<Angular, Header>(num) {}

	void matchDegrees(Array &next);
	virtual void addArray(std::string name, Array next);
protected:
	using DataGroup<Angular, Header>::_length;
	using DataGroup<Angular, Header>::_vectors;
	using DataGroup<Angular, Header>::_diffs;
	using DataGroup<Angular, Header>::_stdevs;

private:

};

#include "DegreeDataGroup.cpp"

#endif
