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

template <typename Storage>
class RTVector
{
public:
	void addResidueTorsion(const ResidueTorsion &rt, 
	                       const Storage &st = Storage{})
	{
		_rtValues.push_back(RTValue{rt, st});
	}
	
	const size_t size() const
	{
		return _rtValues.size();
	}

	void vector_from(const std::vector<ResidueTorsion> &rts)
	{
		_rtValues.clear();
		_rtValues.reserve(rts.size());
		for (const ResidueTorsion &rt : rts)
		{
			addResidueTorsion(rt);
		}
	}
	
	void attachInstance(Instance *inst)
	{
		for (RTValue &val : _rtValues)
		{
			val.rt.attachToInstance(inst);
		}
	}
	
	void vector_from(const std::vector<ResidueTorsion> &rts,
	                 const std::vector<Storage> &storage)
	{
		_rtValues.clear();
		_rtValues.reserve(rts.size());
		assert(rts.size() == storage.size());
		for (int i = 0; i < rts.size(); i++)
		{
			addResidueTorsion(rts.at(i), storage.at(i));
		}
	}
	
	std::vector<ResidueTorsion> rts_only() const
	{
		std::vector<ResidueTorsion> rts;
		rts.reserve(_rtValues.size());
		
		for (const RTValue &val : _rtValues)
		{
			rts.push_back(val.rt);
		}
		
		return rts;
	}
	
	const ResidueTorsion &c_rt(int i) const
	{
		return _rtValues[i].rt;
	}
	
	ResidueTorsion &rt(int i)
	{
		return _rtValues[i].rt;
	}
	
	int indexOfRT(const ResidueTorsion &rt) const
	{
		for (size_t i = 0; i < _rtValues.size(); i++)
		{
			if (_rtValues[i].rt == rt)
			{
				return i;
			}
		}
		
		return -1;
	}
	
	void filter_according_to(const std::vector<Parameter *> &ps)
	{
		std::vector<RTValue> copy = _rtValues;

		_rtValues.clear();
		_rtValues.reserve(ps.size());
		
		for (int i = 0; i < ps.size(); i++)
		{
			int idx = -1;
			for (int j = 0; j < copy.size(); j++)
			{
				if (copy[j].rt.parameter() == ps[i])
				{
					idx = j;
					break;
				}
			}
			
			if (idx >= 0)
			{
				_rtValues.push_back(copy[idx]);
			}
			else
			{
				_rtValues.push_back(RTValue{});
			}
		}
	}
	
	template<typename Different>
	std::vector<Storage> storage_according_to(const RTVector<Different> &other) 
	const
	{
		std::vector<Storage> storage;
		storage.reserve(other.size());
		
		for (int i = 0; i < other.size(); i++)
		{
			const ResidueTorsion &rt = other.c_rt(i);

			int idx = -1;
			if (_rtValues[i].rt == rt)
			{
				idx = i;
			}
			else
			{
			    idx = indexOfRT(rt);
			}
			
			if (idx >= 0)
			{
				storage.push_back(_rtValues[idx].storage);
			}
			else
			{
				storage.push_back(Storage{});
			}
		}
		
		return storage;
	}
	
	std::vector<Storage> storage_only() const
	{
		std::vector<Storage> storage;
		storage.reserve(_rtValues.size());
		
		for (const RTValue &val : _rtValues)
		{
			storage.push_back(val.storage);
		}
		
		return storage;
	}
	
	Storage &storage(int i)
	{
		return _rtValues[i].storage;
	}

	const Storage &c_storage(int i) const
	{
		return _rtValues[i].storage;
	}

	friend void to_json(json &j, const RTVector &value);
	friend void from_json(const json &j, RTVector &value);
protected:
	struct RTValue
	{
		ResidueTorsion rt;
		Storage storage;

	};

	std::vector<RTValue> _rtValues;
};

#endif
