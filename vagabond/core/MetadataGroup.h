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

#ifndef __vagabond__MetadataGroup__
#define __vagabond__MetadataGroup__

#include <vagabond/c4x/DegreeDataGroup.h>
#include <vagabond/core/TorsionRef.h>
#include <vagabond/core/Residue.h>
#include <vagabond/core/ObjectGroup.h>

class Residue;

class HasMetadata;

class MetadataGroup : public ObjectGroup, public DegreeDataGroup<ResidueTorsion>
{
public:
	MetadataGroup(size_t length);
	virtual void addMetadataArray(HasMetadata *hmd, Array next);
	
	virtual void setWhiteList(std::vector<HasMetadata *> list);
	virtual void setSeparateAverage(std::vector<HasMetadata *> list);

	virtual const size_t headerCount() const
	{
		return headers().size();
	}
	
	void clearAverages();
private:

};

#endif

