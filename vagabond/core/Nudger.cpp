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

#include "Nudger.h"

Nudger::Nudger(SpecificNetwork *sn, Mapped<float> *mapped,
               std::atomic<bool> &stop, std::atomic<bool> &skip)
: AltersNetwork(sn, mapped, stop, skip)
{
	_engine = new SimplexEngine(this);
	
}

size_t Nudger::parameterCount()
{
	return _start.size();
}

int Nudger::sendJob(const std::vector<float> &all)
{
	if (_stop)
	{
		throw 0;
	}
	std::vector<float> mod;
	int i = 0;
	for (const float &f : all)
	{
		mod.push_back(f + _start[i]);
		i++;
	}

	_setPoints(mod);
	float sc = _score();
	int ticket = getNextTicket();
	setScoreForTicket(ticket, sc);
	return ticket;
}

void Nudger::nudge(float flex, std::function<float()> score)
{
	_score = score;
	_getPoints(_start);

	std::vector<float> steps = {flex / 2};
	_engine->chooseStepSizes(steps);
	_engine->setMaxRuns(10);
	_engine->start();
}
