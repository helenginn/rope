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

#ifndef __vagabond__TabulatedData__
#define __vagabond__TabulatedData__

#include <string>
#include <vector>
#include <sstream>

class TabulatedData
{
public:
	enum DataType
	{
		Text,
		Number,
	};
	
	typedef std::pair<std::string, std::string> StringPair;
	typedef std::pair<std::string, DataType> HeaderTypePair;
	typedef std::vector<std::string> Strings;

	TabulatedData(const std::vector<HeaderTypePair> &headerTypes);

	void addEntry(const std::vector<StringPair> &entries);
	Strings column(const std::string &header) const;

	size_t longestEntryLength(const std::string &header) const;
	size_t totalWidth(std::vector<size_t> &sizes) const;
	
	size_t entryCount()
	{
		return _entries.size();
	}
	
	std::vector<std::string> entry(int i) const
	{
		return _entries[i];
	}
	
	void order_by(const std::string &header, bool ascending);
	
	std::vector<std::string> headers() const;
	
	friend std::ostream &operator<<(std::ostream &ss, const TabulatedData &data);
private:
	int indexForHeader(const std::string &header) const;
	std::vector<HeaderTypePair> _headerTypes;

	std::vector<Strings> _entries;
};

#endif
