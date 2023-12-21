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

#ifndef __vagabond__Loopy__
#define __vagabond__Loopy__

#include <vector>
#include <string>
#include <thread>
#include "Responder.h"
#include "StructureModification.h"
#include "ParamSet.h"
#include "SimplexEngine.h"

namespace gemmi
{
	struct Structure;
}

class Polymer;
class SequenceComparison;

class Loopy : public HasResponder<Responder<Loopy>>, public StructureModification,
public RunsEngine
{
public:
	Loopy(Polymer *instance);
	
	size_t size()
	{
		return _loops.size();
	}
	
	// add atom positions to Atom "loop" positions
	void operator()(int i);

	virtual size_t parameterCount();
	virtual int sendJob(const std::vector<float> &vals);
	virtual float getResult(int *job_id);
private:
	Polymer *_instance = nullptr;

	void getLoop(SequenceComparison *sc, int start, int end);
	int submitJob(bool show, const std::vector<float> &vals);

	struct Loop
	{
		int start;
		int end;
		std::string seq;
		int offset;
	};

	void copyPositions();
	void writeOutPositions();
	void writeOutPositions(gemmi::Structure &st, int i);

	typedef std::function<void(std::vector<float> &values)> Getter;
	typedef std::function<void(const std::vector<float> &values)> Setter;

	void setGetterSetters(const std::vector<Parameter *> &params,
	                      Getter &getter, Setter &setter, bool main_chain);
	std::vector<float> getSteppers(const std::vector<Parameter *> &params);
	
	void processLoop(Loop &loop);
	void prepareLoop(const Loop &loop);
	void prepareResources();
	float randomiseLoop();
	void grabNewParameters();

	std::vector<Loop> _loops;
	
	struct Active
	{
		Loop *curr = nullptr;
		std::vector<Parameter *> params;
		std::vector<bool> mask;
		std::vector<int> indices;
		std::vector<float> values;
	};

	void randomise(Loopy::Active &active);

	Active _active;
	int _threads = 1;

	Getter _getter{};
	Setter _setter{};
};

#endif
