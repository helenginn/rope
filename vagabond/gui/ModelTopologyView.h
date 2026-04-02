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

#include <vagabond/gui/elements/IndexResponseView.h>
#include <vagabond/gui/elements/Mouse3D.h>
#include <map>

class Box;
class Line;
class Model;
class Chain;
class FloatingText;
class TopologyCircle;
class PositionShifter;

class ModelTopologyView : public Mouse3D, public IndexResponseView
{
public:
	ModelTopologyView(Scene *prev, Model &contents);
	~ModelTopologyView();

	virtual void setup();
	void unhighlight();
	void updateLegend();
	void updateColours();
	
	typedef std::function<void(ModelTopologyView *me,
	                           Chain *chain, const glm::vec3 &where)>
	                            ClickChainEvent;

	typedef std::function<glm::vec3(Chain *chain)> ColouringFunction;
	typedef std::function<Box *()> LegendFunction;
	
	void setClickChainEvent(const ClickChainEvent &event)
	{
		_clickChainEvent = event;
	}
	
	void setColouringFunction(const ColouringFunction &func)
	{
		_colourFunc = func;
	}

	void clicked(TopologyCircle *circle);
	
	void setLegendFunction(const LegendFunction &func)
	{
		_legendFunc = func;
	}
protected:
	virtual void interactedWithNothing(bool left, bool hover);
private:
	void makeShifter();
	void makeDots();
	void addLinks();
	void addCircles();

	Model &_model;
	PositionShifter *_shifter{};
	
	typedef struct 
	{
		Chain *ch;
		glm::vec3 pos;
		FloatingText *text;
	} ChainInfo;
	
	typedef struct 
	{
		TopologyCircle *left;
		TopologyCircle *right;
	} LinkInfo;
	
	glm::vec3 &posForChain(Chain *chain)
	{
		TopologyCircle *fi = _chain2Image[chain];
		return _image2Info[fi].pos;
	}

	std::map<TopologyCircle *, ChainInfo> _image2Info;
	std::map<Chain *, TopologyCircle *> _chain2Image;
	std::map<Line *, LinkInfo> _links;

	ClickChainEvent _clickChainEvent{};
	ColouringFunction _colourFunc{};
	LegendFunction _legendFunc{};
	
	std::vector<Box *> _legends;
};

#endif
