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

#ifndef __vagabond__ChemotaxisEngine__
#define __vagabond__ChemotaxisEngine__

#include "Engine.h"

class Hypersphere;

class ChemotaxisEngine : public Engine
{
public:
	ChemotaxisEngine(RunsEngine *ref);
	virtual ~ChemotaxisEngine();

	virtual void run();
	bool cycle();
private:
	bool tumble();
	bool runner();
	float _step = 0.1;
	float _original = 0.1;

	Hypersphere *_hypersphere = nullptr;
	std::vector<float> _direction;
};

#endif
