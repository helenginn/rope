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

#include "engine/Mechanics.h"
#include "AtomGroup.h"
#include "engine/ForceField.h"
#include "BondCalculator.h"
#include "Sampler.h"

Mechanics::Mechanics(AtomGroup *grp)
{
	_grp = grp;

	if (_grp->connectedGroups().size())
	{
		_grp = grp->connectedGroups()[0];
	}
}

void Mechanics::private_run()
{
	Atom *anchor = _grp->chosenAnchor();
	int dims = 1;
	Sampler sampler(1, dims);
	
	/*
	_forceField = new ForceField(_grp);

	Atom *O1 = _grp->firstAtomWithName("N");
	Atom *C26 = _grp->firstAtomWithName("OXT");

	_forceField->addLength(O1, C26, 3, 0.5);
	*/

	_calculator = new BondCalculator();
	_calculator->setPipelineType(BondCalculator::PipelineAtomPositions);
	_calculator->setMaxSimultaneousThreads(1);
	_calculator->setSampler(&sampler);

	int num = sampler.pointCount();

	_calculator->setTorsionBasisType(TorsionBasis::TypeMechanical);
	_calculator->addAnchorExtension(anchor);
	_calculator->setIgnoreHydrogens(false);
	_calculator->setup();

	_calculator->start();

	int max = 0;
	Result *r = nullptr;
	while (max < 10000)
	{
		Job job{};
		job.custom.allocate_vectors(1, dims, num);
		job.requests = static_cast<JobType>(JobExtractPositions | 
		                                    JobUpdateMechanics);
		_calculator->submitJob(job);
		max++;

		if (r && r->requests & JobExtractPositions)
		{
			r->transplantPositions();
		}
		
		if (r)
		{
			r->destroy();
		}

		r = _calculator->acquireResult();

		if (r == nullptr)
		{
			break;
		}
	}
}
