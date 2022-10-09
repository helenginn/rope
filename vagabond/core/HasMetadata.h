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

#ifndef __vagabond__HasMetadata__
#define __vagabond__HasMetadata__

#include "Metadata.h"
#include <algorithm>

class HasMetadata
{
public:
	virtual ~HasMetadata() {};

	virtual const Metadata::KeyValues metadata() const
	{
		return Metadata::KeyValues();
	}
	
	virtual const std::string id() const = 0;
	
	virtual bool displayable() const = 0;

	template <class C>
	void appendIfMissing(std::vector<C *> &target)
	{
		if (std::find(target.begin(), target.end(), this) != target.end())
		{
			return;
		}

		target.push_back(static_cast<C *>(this));
	}

	template <class C>
	void eraseIfPresent(std::vector<C *> &target)
	{
		typename std::vector<C *>::iterator it;
		while (true)
		{
			it = std::find(target.begin(), target.end(), this);
			if (it == target.end())
			{
				return;
			}
			target.erase(it);
		}
	}

};

#endif
