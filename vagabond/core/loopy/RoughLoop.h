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

#ifndef __vagabond__RoughLoop__
#define __vagabond__RoughLoop__

#include "StructureModification.h"
#include "RunsEngine.h"
#include "Conformer.h"

class Conformer;
class Loopy;

class RoughLoop : public RunsEngine
{
public:
	RoughLoop(Loopy *loopy, Conformer &active);

	std::function<float(Atom *)> calculateWeights();

	virtual int sendJob(const std::vector<float> &vals, Engine *caller);
	virtual float getResult(int *job_id, Engine *caller);

	virtual size_t parameterCount(Engine *caller);
	float roughLoop();
private:
	float refine(const LoopStage &stage);

	int submitJob(bool show, const std::vector<float> &vals);

	Conformer &_active;
	Loopy *_loopy;
	StructureModification::Resources &_resources;
};

#endif
