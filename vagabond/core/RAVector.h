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

#include "Atom3DPosition.h"
#include "HeadedVector.h"
#include "Sequence.h"
#include "Polymer.h"

template <typename Storage>
class RAVector : public HeadedVector<Atom3DPosition, Storage>
{
public:
	using HeadedVector<Atom3DPosition, Storage>::_pairs;
	typedef typename HeadedVector<Atom3DPosition, Storage>::HeaderValue RTValue;

	void addAtom3DPosition(const Atom3DPosition &rt, 
	                       const Storage &st = Storage{})
	{
		_pairs.push_back(RTValue{rt, st});
	}
	
	void vector_from(const std::vector<Atom3DPosition> &ras)
	{
		_pairs.clear();
		_pairs.reserve(ras.size());
		for (const Atom3DPosition &rt : ras)
		{
			addAtom3DPosition(rt);
		}
	}
	
	void attachInstance(Instance *inst)
	{
		if (inst->hasSequence())
		{
			Polymer *pol = static_cast<Polymer *>(inst);
		}

		for (RTValue &val : _pairs)
		{
			val.header.attachToInstance(inst);
		}
	}
	
	void filter_according_to(const std::vector<Atom *> &as)
	{
		std::vector<RTValue> copy = _pairs;

		_pairs.clear();
		_pairs.reserve(as.size());
		
		for (int i = 0; i < as.size(); i++)
		{
			int idx = -1;
			for (int j = 0; j < copy.size() && as[i]; j++)
			{
				if (copy[j].header.fitsAtom(as[i]))
				{
					idx = j;
					break;
				}
			}
			
			if (idx >= 0)
			{
				_pairs.push_back(copy[idx]);
			}
			else
			{
				_pairs.push_back(RTValue{});
			}
		}
	}
	
	void vector_from(const std::vector<Atom3DPosition> &ras,
	                 const std::vector<Storage> &storage)
	{
		_pairs.clear();
		_pairs.reserve(ras.size());
		for (int i = 0; i < ras.size(); i++)
		{
			if (storage.size() > i)
			{
				addAtom3DPosition(ras.at(i), storage.at(i));
			}
			else
			{
				addAtom3DPosition(ras.at(i), Storage{});
			}
		}
	}
	
protected:
};
