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

#ifndef __vagabond__DegreeTypedData__
#define __vagabond__DegreeTypedData__

#include "Angular.h"
#include "TypedData.h"

/** \class DegreeTypedData
 * specialised class of TypedData that takes degree values and ensures all
 * values match previous entries as closely as possible by adding or subtracting
 * multiple of 360 degrees */


template <class Header>
class DegreeTypedData : public TypedData<Angular, Header>
{
public:
	using Array = typename TypedData<Angular, Header>::Array;
	
	DegreeTypedData(int num) : TypedData<Angular, Header>(num) {}

	void matchDegrees(Array &next);
	virtual void addArray(std::string name, Array next);
protected:
	using TypedData<Angular, Header>::_length;
	using TypedData<Angular, Header>::_entries;
	using TypedData<Angular, Header>::_stdevs;

private:

};

#include "DegreeTypedData.cpp"

#endif
