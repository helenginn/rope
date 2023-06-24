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

#ifndef __vagabond__ShowMap__
#define __vagabond__ShowMap__

#include <vagabond/gui/elements/Scene.h>
#include <vagabond/gui/elements/Line.h>
#include <vagabond/core/Responder.h>

class MapView;
class Parameter;
class Cartographer;
class SpecificNetwork;
template <typename Type> class Mapped;

class ShowMap : public Scene, public Responder<SpecificNetwork>
{
public:
	ShowMap(Scene *scene, SpecificNetwork *sn);

	virtual void setup();
	void update(Parameter *param);
	virtual void buttonPressed(std::string tag, Button *button);
	
	void setCartographer(Cartographer *cg)
	{
		_cartographer = cg;
	}
	void setMapView(MapView *mv)
	{
		_view = mv;
	}
private:
	void updateFromMap(Mapped<float> *map);

	SpecificNetwork *_sn = nullptr;
	Parameter *_parameter = nullptr;
	Cartographer *_cartographer = nullptr;
	MapView *_view = nullptr;
	
	int _first = -1;
	int _second = -1;
};

#endif
