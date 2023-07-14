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

#ifndef __vagabond__HeadedVector__
#define __vagabond__HeadedVector__

template <typename Header, typename Storage>
class HeadedVector
{
public:
	template <typename F>
	void do_op(const F &func)
	{
		for (int i = 0; i < size(); i++)
		{
			func(storage(i));
		}
	}

	const size_t size() const
	{
		return _pairs.size();
	}

	std::vector<Header> headers_only() const
	{
		std::vector<Header> headers;
		headers.reserve(_pairs.size());
		
		for (const HeaderValue &val : _pairs)
		{
			headers.push_back(val.header);
		}
		
		return headers;
	}

	std::vector<Storage> storage_only() const
	{
		std::vector<Storage> storage;
		storage.reserve(_pairs.size());
		
		for (const HeaderValue &val : _pairs)
		{
			storage.push_back(val.storage);
		}
		
		return storage;
	}

	const Header &header(int i) const
	{
		return _pairs[i].header;
	}
	
	Header &header(int i)
	{
		return _pairs[i].header;
	}

	int indexOfHeader(const Header &header) const
	{
		for (size_t i = 0; i < _pairs.size(); i++)
		{
			if (_pairs[i].header == header)
			{
				return i;
			}
		}
		
		return -1;
	}

	template<typename Different>
	std::vector<Storage> 
	storage_according_to(const HeadedVector<Header, Different> &other) const
	{
		std::vector<Storage> storage;
		storage.reserve(other.size());
		
		for (int i = 0; i < other.size(); i++)
		{
			const Header &header = other.header(i);

			int idx = -1;
			if (_pairs[i].header == header)
			{
				idx = i;
			}
			else
			{
			    idx = indexOfHeader(header);
			}
			
			if (idx >= 0)
			{
				storage.push_back(_pairs[idx].storage);
			}
			else
			{
				storage.push_back(Storage{});
			}
		}
		
		return storage;
	}
	
	Storage &storage(int i)
	{
		return _pairs[i].storage;
	}

	const Storage &storage(int i) const
	{
		return _pairs[i].storage;
	}
	
protected:
	struct HeaderValue
	{
		Header header;
		Storage storage;

	};

	std::vector<HeaderValue> _pairs;
};

#endif
