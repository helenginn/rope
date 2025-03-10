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

#ifndef __vagabond__Refinement__
#define __vagabond__Refinement__

#include "RefinementInfo.h"
#include <vagabond/core/Responder.h>

#include <list>

class Model;
class Unit;
class Diffraction;
class ArbitraryMap;

class Refinement : public HasResponder<Responder<Refinement>>
{
public:
	Refinement();

	void setModel(Model *model)
	{
		_model = model;
	}
	
	Model *model()
	{
		return _model;
	}
	
	static void run(Refinement *r)
	{
		r->updateMap();
		
		for (size_t i = 0; i < 10; i++)
		{
			r->play();
		}
	}
	
	ArbitraryMap *map()
	{
		return _map;
	}
	
	ArbitraryMap *calculatedMapAtoms(Diffraction **reciprocal = nullptr,
	                                 float max_res = 1.5,
	                                 AtomPosMap *all_atoms = nullptr);
	
	void loadDiffraction(const std::string &filename);
	void setup();
	void play();
	void updateMap();
	void swapMap(ArbitraryMap *map);
private:
	void prepareInstanceDetails();
	Refine::Info &prepareInstance(Instance *mol);
	void setupRefiner(Refine::Info &info);

	Model *_model = nullptr;
	Diffraction *_data = nullptr;
	ArbitraryMap *_map = nullptr;

	std::list<Refine::Info> _molDetails;
	
};

#endif
