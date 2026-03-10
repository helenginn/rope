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

#ifndef __vagabond__ModelTopologyView__
#define __vagabond__ModelTopologyView__

#include <vagabond/gui/elements/Mouse3D.h>
#include <map>

class Model;
class Chain;
class FloatingImage;
class PositionShifter;

class ModelTopologyView : public Mouse3D
{
public:
	ModelTopologyView(Scene *prev, Model &contents);
	~ModelTopologyView();

	virtual void setup();
private:
	void makeShifter();
	void makeDots();
	void addLinks();
	Model &_model;
	PositionShifter *_shifter{};
	
	typedef struct 
	{
		Chain *ch;
		glm::vec3 pos;
	} ChainInfo;

	std::map<FloatingImage *, ChainInfo> _map;

};

#endif
