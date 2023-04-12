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

#ifndef __vagabond__GuiRibbon__
#define __vagabond__GuiRibbon__

#include "GuiRepresentation.h"

class GuiRibbon : public GuiRepresentation
{
public:
	GuiRibbon(GuiAtom *parent);
	virtual ~GuiRibbon();

	void setMulti(bool m);
	virtual void watchAtom(Atom *a);
	void watchBonds(AtomGroup *ag);

	virtual void updateSinglePosition(Atom *a, glm::vec3 &p);
	virtual void updateMultiPositions(Atom *a, WithPos &wp);
	
	virtual void prepareAtomSpace(AtomGroup *ag);
	void convert();

protected:
	virtual void extraUniforms();

private:
	size_t verticesPerAtom();
	size_t indicesPerAtom();
	bool acceptableAtom(Atom *a);
	void convertToCylinder(std::vector<Snow::Vertex> *vertices = nullptr);
	void convertToBezier();
	void insertAtom(Atom *a);

	std::vector<Snow::Vertex> makeBezier(int index);
	void transplantCylinder(std::vector<Snow::Vertex> &cylinder, int start);

	void addCircle(std::vector<Snow::Vertex> &vertices, 
	               glm::vec3 centre, std::vector<glm::vec3> &circle);
	void addCylinderIndices(size_t num);

	bool previousPositionValid();

	std::map<Atom *, int> _atomIndex;
	std::map<Atom *, glm::vec3> _atomPos;
	std::map<int, glm::vec3> _idxPos;

	std::vector<Atom *> _cAlphas;
};

#endif
