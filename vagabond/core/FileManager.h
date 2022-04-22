// vagabond
// Copyright (C) 2019 Helen Ginn
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

#ifndef __vagabond__FileManager__
#define __vagabond__FileManager__

#include <string>
#include <vector>
#include "File.h"

#include <json/json.hpp>
using nlohmann::json;

class FileManagerResponder
{
public:
	virtual ~FileManagerResponder() {};
	virtual void filesChanged() = 0;
};

class FileManager
{
public:
	FileManager();

	void setFilterType(File::Type type);
	bool valid(std::string filename);

	void setFileView(FileManagerResponder *fileView)
	{
		_view = fileView;
		setFilterType(File::Nothing);
	}

	bool acceptFile(std::string filename, bool force = false);
	
	const size_t filteredCount() const
	{
		return _filtered.size();
	}

	std::string filtered(int i)
	{
		return _filtered[i];
	}

	void addFile(std::string filename);

	friend void to_json(json &j, const FileManager &value);
	friend void from_json(const json &j, FileManager &value);
private:
	std::vector<std::string> _list;
	std::vector<std::string> _filtered;

	File::Type _type = File::Nothing;
	FileManagerResponder *_view;
};

inline void to_json(json &j, const FileManager &manager)
{
	j["files"] = manager._list;
}

inline void from_json(const json &j, FileManager &manager)
{
	j.at("files").get_to(manager._list);
}


#endif
