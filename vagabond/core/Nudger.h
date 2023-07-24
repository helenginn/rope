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

#ifndef __vagabond__Nudger__
#define __vagabond__Nudger__

#include "AltersNetwork.h"
#include "SimplexEngine.h"

class Nudger : public AltersNetwork, public RunsEngine
{
public:
	Nudger(SpecificNetwork *sn, Mapped<float> *mapped,
	         std::atomic<bool> &stop, std::atomic<bool> &skip);

	void nudge(std::vector<float> &flex, std::function<float()> score);

	virtual size_t parameterCount();
	virtual int sendJob(const std::vector<float> &all);
	
	void setBest(float b)
	{
		_best = b;
	}
private:
	SimplexEngine *_engine = nullptr;
	std::function<float()> _score;
	std::vector<float> _start; 
	std::vector<float> _improved; 
	
	int _counter = 0;
	float _best = FLT_MAX;
};

#endif
