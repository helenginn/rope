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
#include "Engine.h"
#include <vagabond/utils/svd/PCA.h>

class Warp;
class ArbitraryMap;

class MolRefiner : public StructureModification, public RunsEngine
{
public:
	MolRefiner(ArbitraryMap *comparison, Refine::Info *info, int num, int dims);

	void submitJob(std::vector<float> mean, std::vector<float> tensor,
	               bool show = true);
	void retrieveJobs();
	void runEngine();

	virtual int sendJob(const std::vector<float> &all);
	virtual float getResult(int *job_id);
	virtual size_t parameterCount();
	void addToMap(ArbitraryMap *map);
protected:
	virtual void customModifications(BondCalculator *calc, bool has_mol);
private:
	std::vector<float> findTensorAxes(std::vector<float> &triangle);
	void triangle_to_svd(std::vector<float> &triangle);
	ArbitraryMap *_map = nullptr;
	Refine::Info *_info = nullptr;
	Warp *_warp = nullptr;

	std::map<int, int> _ticket2Group;
	std::map<int, float> _group2Scores;
	int _groupTicketCount = 0;
	
	std::vector<float> _best;

	bool _getSegment = false;
	PCA::SVD _svd;
};

#endif
