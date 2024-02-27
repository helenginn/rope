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

#ifndef __vcagabond__Points__
#define __vcagabond__Points__

#include <list>
#include <map>
#include <vagabond/gui/elements/IndexResponder.h>

class Atom;
class Visual;
class Untangle;

class Points : public IndexResponder
{
public:
	Points(Visual *visual);
	
	void addAtom(Atom *atom);

	virtual size_t requestedIndices()
	{
		return _atoms.size();
	}
	
	void repositionAtoms();

	virtual void reindex();
	virtual void interacted(int idx, bool hover, bool left);

	virtual void extraUniforms();
private:
	void switchConfs(Atom *a, const std::string &l, const std::string &r,
	                 bool one_only);
	void colourAtom(Vertex &vertex, const std::string &conf);
	void repositionAtom(Atom *atom);
	
	std::vector<std::pair<Atom *, std::string> > _atoms;
	std::map<Atom *, std::vector<int>> _map;
	Visual *_visual = nullptr;
	Untangle *_untangle = nullptr;
	float _size = 1;

};

#endif
