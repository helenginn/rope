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

#ifndef __vagabond__GuiBalls__
#define __vagabond__GuiBalls__

#include <vagabond/gui/elements/IndexResponder.h>

#include "GuiRepresentation.h"

class Display;
class GuiAtom;
class GuiBond;
class FloatingText;

class GuiBalls : public GuiRepresentation, 
public HasResponder<Responder<GuiBalls>>
{
public:
	GuiBalls(GuiAtom *parent);
	virtual ~GuiBalls();
	
	void setDisplay(Display *scene)
	{
		_scene = scene;
	}

	virtual void watchAtom(Atom *a);
	void watchBonds(AtomGroup *ag);

	virtual size_t requestedIndices()
	{
		return _atomIndex.size();
	}

	virtual void reindex();
	virtual void interacted(int idx, bool hover, bool left);

	void setMulti(bool m);

	virtual void updateSinglePosition(Atom *a, glm::vec3 &p);
	virtual void updateMultiPositions(Atom *a, WithPos &wp);
	
	virtual void prepareAtomSpace(AtomGroup *ag);
	virtual void addVisuals(Atom *a);

	virtual void removeVisuals();
	virtual void finishUpdate();
protected:
	virtual void extraUniforms();
private:
	size_t verticesPerAtom();
	size_t indicesPerAtom();

	virtual void selected(int idx, bool inverse);
	
	virtual bool selectable() const
	{
		return true;
	}
	
	void deselect();

	void colourByElement(std::string ele);
	void setPosition(glm::vec3 position);

	Renderable *_template = nullptr;
	GuiBond *_bonds = nullptr;
	
	Display *_scene = nullptr;

	std::map<Atom *, int> _atomIndex;
	std::map<int, Atom *> _indexAtom;
	std::map<Atom *, glm::vec3> _atomPos;
	
	bool _multi = false;
	FloatingText *_text = nullptr;
	
	float _size = 30;
	int _lastIdx = -1;
};

#endif
