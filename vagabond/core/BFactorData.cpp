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

#include "BFactorData.h"
#include "HasMetadata.h"

void BFactorData::addMetadataArray(HasMetadata *hmd, Array next)
{
	_objects.push_back(hmd);
	std::string name = hmd->id();
	this->addArray(name, next);
}

BFactorData::~BFactorData()
{
	
}

const RAFloats &BFactorData::emptyBFactors(bool make_nan) 
{
	if (_empty.size() == 0)
	{
		std::vector<Atom3DPosition> hs = headers();
		_empty.vector_from(hs);
	}

	float zero = make_nan ? NAN : 0;
	
	for (size_t i = 0; i < _empty.size(); i++)
	{
		_empty.storage(i) = zero;
	}
	
	return _empty;
}
