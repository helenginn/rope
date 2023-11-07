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

#ifndef __vagabond__MolRefiner__
#define __vagabond__MolRefiner__

#include "StructureModification.h"
#include "RefinementInfo.h"
#include "Translation.h"
#include "Engine.h"
#include <vagabond/utils/svd/PCA.h>
#include <vagabond/core/Sampler.h>

class Warp;
class ArbitraryMap;

class MolRefiner : public StructureModification, public RunsEngine
{
public:
	MolRefiner(ArbitraryMap *comparison, Refine::Info *info);
	~MolRefiner();
	
	void changeMap(ArbitraryMap *map)
	{
		_map = map;
	}

	void retrieveJobs();
	void runEngine();

	Result *submitJobAndRetrieve(const std::vector<float> &all);
	virtual int sendJob(const std::vector<float> &all);
	virtual float getResult(int *job_id);
	virtual size_t parameterCount();

	typedef std::function<void(std::vector<float> &values)> Getter;
	typedef std::function<void(const std::vector<float> &values)> Setter;
	virtual void prepareResources();
protected:
	void changeDefaults(CoordManager *manager);
private:
	void submitJob(std::vector<float> all);
	void calculate(const std::vector<float> &params);
	void setGetterSetters();

	float confParams(int n);

	ArbitraryMap *_map = nullptr;
	Refine::Info *_info = nullptr;

	int _ticket = 0;
	
	Floats _best;
	Sampler _sampler;
	Translation _translate;

	Getter _getter{};
	Setter _setter{};
	
	Floats _parameters;
};

#endif
