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

#ifndef __vagabond__PositionalGroup__
#define __vagabond__PositionalGroup__

#include "Residue.h"
#include <vagabond/c4x/Posular.h>
#include <vagabond/c4x/DataGroup.h>
#include <vagabond/core/ObjectGroup.h>

class PositionalGroup : public ObjectGroup, 
public DataGroup<Posular, Atom3DPosition>
{
public:
	PositionalGroup(size_t length) : DataGroup<Posular, Atom3DPosition>(length)
	{
		
	}

	virtual void addMetadataArray(HasMetadata *hmd, Array next);
	virtual void setWhiteList(std::vector<HasMetadata *> list);

	virtual const int groupCount() const
	{
		return DataGroup<Posular, Atom3DPosition>::groupCount();
	}

	virtual const size_t headerCount() const
	{
		return headers().size();
	}

	virtual void clearAverages()
	{
		return DataGroup<Posular, Atom3DPosition>::clearAverages();
	}
private:

};


#endif
