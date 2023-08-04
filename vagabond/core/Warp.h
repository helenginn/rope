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

#ifndef __vagabond__Warp__
#define __vagabond__Warp__

#include "StructureModification.h"
#include "PositionSampler.h"
#include <vagabond/core/Responder.h>
#include <vagabond/utils/AcquireCoord.h>
#include "AtomPosMap.h"

class Instance;
class TorsionBasis;
class BondSequence;
class CompareDistances;

class Warp : public StructureModification, public PositionSampler,
public HasResponder<Responder<Warp>>
{
public:
	Warp(Instance *ref, size_t num_axes);

	const int &numAxes() const
	{
		return _dims;
	}

	void setup();
	
	std::function<float()> score(const std::vector<Floats> &points);

	int submitJob(bool show, const std::vector<float> &vals);

	virtual bool prewarnAtoms(BondSequence *bc, const Coord::Get &get,
	                          Vec3s &positions);

	virtual void prewarnBonds(BondSequence *seq, const Coord::Get &get,
	                          Floats &torsions);
	
	void setAtomMotions(std::function<Vec3s(const Coord::Get &)> &func)
	{
		_atom_positions_for_coord = func;
	}
	
	void setBondMotions(std::function<Floats(const Coord::Get &)> &func)
	{
		_torsion_angles_for_coord = func;
	}
	
	const std::vector<Parameter *> &parameterList() const
	{
		return _base.parameters;
	}
	
	const std::vector<Atom *> &atomList() const
	{
		return _base.atoms;
	}
	
	void clearComparison();
	void clearFilters();
	
	typedef std::function<bool(Atom *const &atom)> AtomFilter;

	void setCompareFilters(AtomFilter &left, AtomFilter &right);

	CompareDistances *compare();

	void exposeDistanceMatrix();
protected:
	
	virtual bool handleAtomList(AtomPosList &list);
private:
	void prepareAtoms();
	void prepareBonds();

	int _dims = 0;
	int _jobNum = 0;

	std::function<Vec3s(const Coord::Get &)> _atom_positions_for_coord{};
	std::function<Floats(const Coord::Get &)> _torsion_angles_for_coord{};

	AtomFilter _filter;

	struct BasePositions
	{
		std::vector<Atom *> atoms;
		Vec3s positions;

		std::vector<Parameter *> parameters;
		Floats torsions;
	};
	
	BasePositions _base;
	CompareDistances *_compare = nullptr;
	
	PCA::Matrix _distances;
};

#endif
