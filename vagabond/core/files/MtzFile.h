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

#ifndef __vagabond__MtzFile__
#define __vagabond__MtzFile__

#include "../../utils/os.h"
#ifdef OS_WINDOWS
	#define NOMINMAX  // avoid conflict with std::min and std::max in gemmi
#endif

#include <functional>
#include "File.h"

class Diffraction;
class ArbitraryMap;

namespace gemmi
{
	struct Mtz;
}

struct WriteColumn
{
	template <class Func>
	WriteColumn(const std::string &_name, const std::string &_type,
	            Func value)
	{
		name = _name;
		type = _type;
		get_value = value;
	}
	
	float operator()(int i, int j, int k) const
	{
		return get_value(i, j, k);
	}

	std::string name;
	std::string type;
	std::function<float(int, int, int)> get_value;
};

class MtzFile : public File
{
public:
	MtzFile(std::string filename = "");
	MtzFile(Diffraction *diffraction);
	
	void setMap(Diffraction *diffraction)
	{
		_map = diffraction;
	}

	void setMap(ArbitraryMap *map);

	std::string write_to_string(float max_res = -1,
	                            std::vector<WriteColumn> columns = {});
	void write_to_file(std::string filename, float max_res = -1,
	                   std::vector<WriteColumn> columns = {});
	virtual File::Type cursoryLook();
	virtual void parse();
private:
	gemmi::Mtz prep_gemmi_mtz(float max);
	gemmi::Mtz prepare_mtz(float max_res, const std::vector<WriteColumn> &columns);

	Diffraction *_map = nullptr;
};

#endif
