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

#ifndef __vagabond__Wiggler__
#define __vagabond__Wiggler__

#include "RefinementInfo.h"

class Instance;
class Sampler;

namespace Refine {
	struct Info;
};

class Wiggler
{
public:
	Wiggler(Refine::Info &info, Sampler *sampler);
	virtual ~Wiggler() {};
	
	void setModules(const Refine::Module &mod)
	{
		_modules = mod;
		_info.modules = mod;
	}
	
	void operator()();
	
	int n_params();
	std::vector<int> chopped_params();

	int confParams();
	int transParams();
	int rotParams();
	int baryParams();
	
	void setSuperpose(bool superpose)
	{
		_superpose = superpose;
	}

	AtomGroup *group();

	Refine::Calculate prepareSubmission();

	Refine::Info &_info;
	Sampler *_sampler = nullptr;
	
	bool _superpose = true;
	
	Refine::Module _modules = Refine::None;
};

#endif
