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

#include "TabulatedData.h"
#include <iostream>
#include <iomanip>
#include <algorithm>

TabulatedData::TabulatedData(const std::vector<HeaderTypePair> &headerTypes)
: _headerTypes(headerTypes)
{

}

void TabulatedData::addEntry(const std::vector<StringPair> &entries)
{
	std::vector<StringPair> copy = entries;
	Strings entry;
	
	for (const HeaderTypePair &headerType : _headerTypes)
	{
		bool found = false;
		int i = 0;
		for (const StringPair &pair : copy)
		{
			if (headerType.first == pair.first)
			{
				entry.push_back(pair.second);
				copy.erase(copy.begin() + i);
				found = true;
				break;
			}

			i++;
		}
		
		if (!found)
		{
			entry.push_back("");
		}
	}
	
	_entries.push_back(entry);
	
	for (const StringPair &pair : copy)
	{
		std::cout << "Warning: did not add " << pair.first << " entry "
		<< pair.second << " to table, unknown header." << std::endl;
	}
}

std::vector<float> cast_to_floats(const TabulatedData::Strings &strings)
{
	std::vector<float> floats;
	floats.reserve(strings.size());

	for (const std::string &str : strings)
	{
		floats.push_back(atof(str.c_str()));
	}
	
	return floats;
}

template <typename Type>
void sort_by(std::vector<TabulatedData::Strings> &entries, 
             const std::vector<Type> &column, bool ascending)
{
	if (entries.size() != column.size())
	{
		throw std::runtime_error("Some unexpected size comparison in table "\
		                         "ordering occurred.");
	}

	typedef std::pair<TabulatedData::Strings, Type> Pair;

	std::vector<Pair> pairs; pairs.reserve(entries.size());
	
	for (size_t i = 0; i < entries.size(); i++)
	{
		pairs.push_back({entries[i], column[i]});
	}

	auto compare = [ascending](const Pair &a, const Pair &b)
	{
		return (ascending ? a.second > b.second : a.second < b.second);
	};

	std::sort(pairs.begin(), pairs.end(), compare);
	
	for (size_t i = 0; i < entries.size(); i++)
	{
		entries[i] = pairs[i].first;
	}
}

void TabulatedData::order_by(const std::string &header, bool ascending)
{
	int idx = indexForHeader(header);
	Strings col = column(header);

	if (_headerTypes[idx].second == Number)
	{
		std::vector<float> vals = cast_to_floats(col);
		sort_by(_entries, vals, ascending);
	}
	else
	{
		sort_by(_entries, col, ascending);
	}
}

int TabulatedData::indexForHeader(const std::string &header) const
{
	int i = 0;
	for (const HeaderTypePair &headerType : _headerTypes)
	{
		if (headerType.first == header)
		{
			return i;
		}
		i++;
	}

	throw std::runtime_error("Header " + header + "requested not present "\
	                         "in table");
}

std::vector<std::string> TabulatedData::headers() const
{
	std::vector<std::string> headers;

	for (const HeaderTypePair &headerType : _headerTypes)
	{
		headers.push_back(headerType.first);
	}

	return headers;
}

TabulatedData::Strings TabulatedData::column(const std::string &header) const
{
	int idx = indexForHeader(header);
	
	Strings column;
	
	for (const Strings &entry : _entries)
	{
		column.push_back(entry[idx]);
	}
	
	return column;
}

size_t TabulatedData::longestEntryLength(const std::string &header) const
{
	int idx = indexForHeader(header);
	
	size_t length = _headerTypes[idx].first.length();

	for (const Strings &entry : _entries)
	{
		size_t candidate = entry[idx].length();
		length = std::max(candidate, length);
	}
	
	return length;
}

size_t TabulatedData::totalWidth(std::vector<size_t> &sizes) const
{
	size_t length = 0;
	sizes.clear();

	for (int i = 0; i < _headerTypes.size(); i++)
	{
		size_t l = longestEntryLength(_headerTypes[i].first);
		length += l;
		length += 3; // barrier between entries
		sizes.push_back(l);
	}

	return length;
}

std::ostream &operator<<(std::ostream &ss, const TabulatedData &data)
{
	auto print_words = [&ss](const std::vector<size_t> &sizes,
	                        const TabulatedData::Strings &strings)
	{
		if (sizes.size() != strings.size())
		{
			throw std::runtime_error("Error in corresponding sizes printing "\
			                         "table");
		}

		ss << "  | ";
		for (int i = 0; i < sizes.size(); i++)
		{
			ss << std::setw(sizes[i]) << strings[i] << " | ";
		}
		ss << std::endl;
	};

	auto print_character = [&ss](unsigned char ch, int count)
	{
		ss << "  ";
		for (int i = 0 ; i < count; i++)
		{
			ss << ch;
		}
		ss << std::endl;
	};

	size_t pad = 1;
	std::vector<size_t> sizes;
	size_t width = data.totalWidth(sizes);
	print_character('=', width + pad);
	print_words(sizes, data.headers());
	print_character('-', width + pad);
	
	for (const TabulatedData::Strings &entry : data._entries)
	{
		print_words(sizes, entry);
	}

	print_character('=', width + pad);

	return ss;
}
