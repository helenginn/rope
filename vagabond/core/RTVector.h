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

#ifndef __vagabond__RTVector__
#define __vagabond__RTVector__

#include "ResidueTorsion.h"
#include "HeadedVector.h"

template <typename Storage>
class RTVector : public HeadedVector<ResidueTorsion, Storage>
{
public:
	using HeadedVector<ResidueTorsion, Storage>::_pairs;
	typedef typename HeadedVector<ResidueTorsion, Storage>::HeaderValue RTValue;

	void addResidueTorsion(const ResidueTorsion &rt, 
	                       const Storage &st = Storage{})
	{
		_pairs.push_back(RTValue{rt, st});
	}
	
	void vector_from(const std::vector<ResidueTorsion> &rts)
	{
		_pairs.clear();
		_pairs.reserve(rts.size());
		for (const ResidueTorsion &rt : rts)
		{
			addResidueTorsion(rt);
		}
	}
	
	void attachInstance(Instance *inst)
	{
		for (RTValue &val : _pairs)
		{
			val.header.attachToInstance(inst);
		}
	}
	
	void vector_from(const std::vector<ResidueTorsion> &rts,
	                 const std::vector<Storage> &storage)
	{
		_pairs.clear();
		_pairs.reserve(rts.size());
		for (int i = 0; i < rts.size(); i++)
		{
			if (storage.size() > i)
			{
				addResidueTorsion(rts.at(i), storage.at(i));
			}
			else
			{
				addResidueTorsion(rts.at(i), Storage{});
			}
		}
	}
	
	const ResidueTorsion &c_rt(int i) const
	{
		return _pairs[i].header;
	}
	
	ResidueTorsion &rt(int i)
	{
		return _pairs[i].header;
	}
	
	void filter_according_to(const std::vector<Parameter *> &ps)
	{
		std::vector<RTValue> copy = _pairs;

		_pairs.clear();
		_pairs.reserve(ps.size());
		
		for (int i = 0; i < ps.size(); i++)
		{
			int idx = -1;
			for (int j = 0; j < copy.size(); j++)
			{
				if (copy[j].header.parameter() == ps[i])
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
	
	
protected:
};


#endif
