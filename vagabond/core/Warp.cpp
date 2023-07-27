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

#include "BondSequence.h"
#include "Instance.h"
#include "Warp.h"

Warp::Warp(Instance *ref, size_t num_axes) 
: StructureModification(ref,  1, num_axes)
{
	_dims = num_axes;
}

int Warp::submitJob(bool show, const std::vector<float> vals)
{
	for (BondCalculator *calc : _calculators)
	{
		Job job{};
		job.custom.allocate_vectors(1, _dims, _num);
		job.pos_sampler = this;

		for (size_t i = 0; i < vals.size(); i++)
		{
			job.custom.vecs[0].mean[i] = vals[i];
		}

		job.requests = static_cast<JobType>(JobPositionVector |
		                                    JobCalculateDeviations);
		if (!show)
		{
			job.requests = JobCalculateDeviations;
		}

		int t = calc->submitJob(job);
		_ticket2Point[t] = _jobNum;
	}

	_point2Score[_jobNum] = Score{};
	_jobNum++;
	return _jobNum - 1;
}

void Warp::setup()
{
	_base = {};
	startCalculator();
	prepareAtoms();
	prepareBonds();
	
	_filter = [](Atom *const &atom)
	{
		return (atom->atomName() == "CA");
	};
	
	_displayTargets = true;
}

bool Warp::prewarnAtoms(BondSequence *bc, const Coord::Get &get, Vec3s &ps)
{
	// set ps to the atoms for this get, return true (always valid)
	ps = _atom_positions_for_coord(get);
	ps += _base.positions;

	return true;
}

void Warp::prewarnBonds(BondSequence *seq, const Coord::Get &get, Floats &ts)
{
	// set ts to the torsion angles for this get
	ts = _torsion_angles_for_coord(get);
}

void Warp::prepareAtoms()
{
	BondCalculator *calc = _instanceToCalculator[_instance];
	BondSequence *seq = calc->sequence();

	const std::vector<AtomBlock> &blocks = seq->blocks();

	for (int i = 0; i < blocks.size(); i++)
	{
		Atom *search = blocks[i].atom;
		_base.atoms.push_back(search);

		if (!search)
		{
			_base.positions.push_back(glm::vec3(NAN));
			continue;
		}

		glm::vec3 pos = search->derivedPosition();
		_base.positions.push_back(pos);
	}
	
	Floats &torsions = _base.torsions;
	_torsion_angles_for_coord = [torsions](const Coord::Get &get)
	{
		return torsions;
	};
}	

void Warp::prepareBonds()
{
	BondCalculator *calc = _instanceToCalculator[_instance];
	BondSequence *seq = calc->sequence();

	TorsionBasis *basis = seq->torsionBasis();
	_base.torsions.resize(basis->parameterCount());
	int i = 0;
	for (float &f : _base.torsions)
	{
		_base.parameters.push_back(basis->parameter(i));
		f = basis->parameter(i)->value();
		i++;
	}
	
	Vec3s positions;
	positions.resize(_base.positions.size());

	_atom_positions_for_coord = [positions](const Coord::Get &get)
	{
		return positions;
	};
}
