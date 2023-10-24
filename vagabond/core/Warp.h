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
class TorsionWarp;

class Warp : public StructureModification, public PositionSampler,
public HasResponder<Responder<Warp>>
{
public:
	Warp(Instance *ref, size_t num_axes);
	
	static Warp *warpFromFile(Instance *reference, std::string file);

	const int &numAxes() const
	{
		return _dims;
	}

	void setup();
	
	std::function<float()> score(const std::vector<Floats> &points);

	int submitJob(bool show, const std::vector<float> &vals);

	virtual glm::vec3 prewarnAtom(BondSequence *bc, const Coord::Get &get,
	                              int index);
	virtual bool prewarnAtoms(BondSequence *bc, const Coord::Get &get,
	                          Vec3s &positions);

	virtual float prewarnBond(BondSequence *bc, const Coord::Get &get,
	                              int index);
	virtual void prewarnBonds(BondSequence *seq, const Coord::Get &get,
	                          Floats &torsions);
	
	void setAtomMotions(std::function<glm::vec3(const Coord::Get &, int num)> &func)
	{
		_atom_positions_for_coord = func;
	}
	
	void setBondMotions(std::function<float(const Coord::Get &, int num)> &func)
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
	void resetComparison();
	void clearFilters();
	
	typedef std::function<bool(Atom *const &atom)> AtomFilter;
	
	void setTorsionWarp(TorsionWarp *tw)
	{
		_tWarp = tw;
	}
	
	TorsionWarp *torsionWarp()
	{
		return _tWarp;
	}

	CompareDistances *compare();

	void exposeDistanceMatrix();
	
	void setShowMatrix(bool show)
	{
		_showMatrix = show;
	}

	void cleanup();
	void saveSpace(const std::string &filename);
protected:
	virtual void customModifications(BondCalculator *calc, bool has_mol = true);
	
	virtual bool handleAtomList(AtomPosList &list);

private:
	void prepareAtoms();
	void prepareBonds();

	int _dims = 0;
	int _jobNum = 0;

	std::function<glm::vec3(const Coord::Get &, int num)> _atom_positions_for_coord{};
	std::function<float(const Coord::Get &, int num)> _torsion_angles_for_coord{};

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
	TorsionWarp *_tWarp = nullptr;
	
	PCA::Matrix _distances;
	int _count = 0;
	bool _alwaysShow = true;
	bool _showMatrix = true;
};

#endif
