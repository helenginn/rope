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

#ifndef __vagabond__ObjectData__
#define __vagabond__ObjectData__

#include <vector>
#include <string>
#include <vagabond/c4x/Data.h>
#include <vagabond/c4x/Cluster.h>

class HasMetadata;
class Instance;
class Rule;
class Path;
class Menu;
class Axes;

class ObjectData
{
public:
	ObjectData(Data *data)
	{
		_data = data;
	}

	virtual ~ObjectData() {};

	std::vector<float> numbersForKey(std::string key);

	virtual void setSeparateAverage(std::vector<HasMetadata *> list);

	std::vector<HasMetadata *> subsetFromRule(const Rule &r);

	bool coversPath(Path *path);

	virtual void setWhiteList(std::vector<HasMetadata *> list);

	virtual void setWhiteList(std::vector<Instance *> list);

	const size_t objectCount() const
	{
		return _objects.size();
	}
	
	size_t numGroups() const;

	virtual void write_data(std::string filename);
	
	std::string csvFirstLine();
	
	virtual std::string header_for_entry_in_csv_file() = 0;
	virtual std::vector<std::string> columns_for_entry_in_csv_file() = 0;
	virtual std::string y_axis_label() = 0;

	void rawVectorToCSV(Cluster *cluster, int axis_idx, std::ostream &ss);
	
	const int indexOfObject(HasMetadata *obj) const
	{
		for (size_t i = 0; i < _objects.size(); i++)
		{
			if (obj == _objects[i])
			{
				return i;
			}
		}

		return -1;
	}
	
	virtual void clearAverages();
	
	void editMenu(Axes *axes, Menu *menu);
	void doRequest(Axes *axes, const std::string &request);
	
	typedef std::function<void(Axes *, Menu *)> DoEditMenu;
	typedef std::function<void(Axes *, const std::string &)> DoRequest;
	
	void setDoEditMenu(const DoEditMenu &do_it)
	{
		_doEditMenu = do_it;
	}
	
	void setDoRequest(const DoRequest &do_it)
	{
		_doRequest = do_it;
	}

	HasMetadata *const object(int i) 
	{
		return _objects[i];
	}
	
	const std::vector<HasMetadata *> &objects() const
	{
		return _objects;
	}
	
	Data *const &data()
	{
		return _data;
	}
	
	std::vector<Instance *> asInstances() const;

	bool purgeObject(HasMetadata *hm);
protected:
	Data *_data = nullptr;

	std::vector<HasMetadata *> _objects;

	DoEditMenu _doEditMenu;
	DoRequest _doRequest;
};

#endif
