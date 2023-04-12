// vagabond
// Copyright (C) 2019 Helen Ginn
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

#ifndef __vagabond__GuiBond__
#define __vagabond__GuiBond__

#include <vagabond/gui/elements/Renderable.h>
#include <vagabond/core/Atom.h>

class AtomGroup;

class GuiBond : public Renderable
{
public:
	GuiBond();
	~GuiBond();
	
	void interacted(int idx, bool hover, bool left) {}
	void reindex();
	size_t requestedIndices()
	{
		return 0;
	}

	void watchBonds(AtomGroup *a);
	
	/** update atom position
	 * @param a atom object
	 * @param pos new position in space
	 * @param n conformer number within ensemble */
	void updateAtom(Atom *a, glm::vec3 pos, int n = 0);
	void updateAtoms(Atom *a, WithPos &wp);
	
	void addVisuals(Atom *a);
	void removeVisuals();
	virtual void render(SnowGL *gl);
private:
	void changeNetworks(int n);
	void incrementNetworks(int n);
	void truncateNetworks(int n);

	std::map<Atom *, size_t> _atomIdx;
	Renderable *_copy = nullptr;

	int _num = 0;
};

#endif
