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
#include <vagabond/core/RopeTypes.h>  

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

	struct Watch
	{
		struct Entry
		{
			Atom *atom = nullptr;
			glm::vec3 pos = glm::vec3(NAN, NAN, NAN);
			// next_idx refers to the first index in the next stage's array
			// belonging to this atom.
			int next_idx = -1;
			int radius = 1;

		};
		
		Atom *lastAtom()
		{
			if (_atoms.size() == 0)
			{
				return nullptr;
			}

			return _atoms.back().atom;
		}
		
		int stopIndex(int idx)
		{
			int j = idx + 1;

			while (_atoms[j].next_idx < 0 && j < _atoms.size() - 1)
			{
				j++;
			}

			return _atoms[j].next_idx;
		}
		
		void setNextIndex(int idx, int next)
		{
			 if (_atoms[idx].atom != nullptr && _atoms[idx].next_idx < 0)
			{
				_atoms[idx].next_idx = next;
			}
		}
		
		const size_t size() const
		{
			return _atoms.size();
		}

		void addWatchedAtom(const Entry &entry)
		{
			if (entry.atom)
			{
				_atom2Idx[entry.atom] = _atoms.size();
			}

			_atoms.push_back(entry);
		}
		
		const Entry &operator[](int idx) const
		{
			return _atoms.at(idx);
		}

		bool has(Atom *atom) const
		{
			return _atom2Idx.count(atom);
		}
		
		const int index(Atom *atom) const
		{
			return _atom2Idx.at(atom);
		}
		
		const Entry &operator[](Atom *atom) const
		{
			int idx = _atom2Idx.at(atom);
			return _atoms.at(idx);
		}
		
		Entry &operator[](int idx)
		{
			return _atoms.at(idx);
		}
		
		Entry &operator[](Atom *atom) 
		{
			int idx = _atom2Idx.at(atom);
			return _atoms.at(idx);
		}
		
		std::vector<Entry> _atoms;
		std::map<Atom *, size_t> _atom2Idx;
	};
	
protected:
	virtual void extraUniforms();

private:
	size_t verticesPerAtom();
	size_t indicesPerAtom();
	bool acceptableAtom(Atom *a);
	void prepareCylinder();
	void prepareCylinder(int i);
	std::vector<Vertex> verticesForCylinder(int i);
	void prepareBezier();
	void prepareBezier(int i);
	void insertAtom(Atom *a);

	std::vector<Snow::Vertex> makeBezier(int index);
	void transplantCylinder(std::vector<Snow::Vertex> &cylinder, int start);

	std::vector<Snow::Vertex> addCircle(glm::vec3 centre, 
	                                    const std::vector<glm::vec3> &circle);
	void addCylinderIndices(size_t num);

	bool previousPositionValid();
	
	/* _atoms will contain one entry per atom (and nullptrs to end chains). */
	Watch _atoms;
	
	/* _bezier will contain INDICES_PER_BEZIER_DIVISION indices per atom. */
	Watch _bezier;

	bool correct_indices(int *is, GuiRibbon::Watch &atoms);
	void colourCylinderVertices(std::vector<Vertex> &vertices, Atom *a);
	std::vector<glm::vec3> bezierSegment(glm::vec3 p1, glm::vec3 p2,
	                                     glm::vec3 p3, glm::vec3 p4);
	void segmentToWatch(std::vector<glm::vec3> &segment,
	                    GuiRibbon::Watch::Entry &source,
	                    GuiRibbon::Watch &watch);

	std::map<Atom *, int> _atomIndex;
	std::map<Atom *, glm::vec3> _atomPos;
	std::map<int, glm::vec3> _idxPos;

	std::vector<Atom *> _cAlphas;
	AtomGroup * _group;
};

#endif
