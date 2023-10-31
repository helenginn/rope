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

#include <vagabond/utils/include_boost.h>
#include <fstream>

#include <vagabond/core/PdbFile.h>
#include <vagabond/core/AtomGroup.h>
#include <vagabond/core/Sequence.h>
#include <vagabond/core/BondSequence.h>
#include <vagabond/core/BondCalculator.h>

BOOST_AUTO_TEST_CASE(ramachandran_plot)
{
	return;
	Sequence seq("aaa");
	AtomGroup *grp = seq.convertToAtoms();
	
	Atom *anchor = grp->chosenAnchor();
	Atom *ca = grp->atomByIdName(ResidueId(2), "CA");
	Atom *c = grp->atomByIdName(ResidueId(2), "C");

	BondCalculator calculator;
	calculator.setPipelineType(BondCalculator::PipelineForceField);
	calculator.setMaxSimultaneousThreads(1);
	calculator.setTotalSamples(1);
	calculator.setTorsionBasisType(TorsionBasis::TypeSimple);
	calculator.setSuperpose(false);
	calculator.addAnchorExtension(anchor);

	calculator.setup();

	calculator.start();
	
	BondSequence *s = calculator.sequence();
	const Grapher &grapher = s->grapher();
	
	AtomGraph *gr_ca = grapher.graph(ca);
	AtomGraph *gr_c = grapher.graph(c);
	BondTorsion *t_phi = gr_ca->torsion;
	BondTorsion *t_psi = gr_c->torsion;

	t_phi->setRefinedAngle(0.f);
	t_psi->setRefinedAngle(0.f);
	
	std::cout << t_phi->desc() << " " << t_psi->desc() << std::endl;
	
	TorsionBasis *basis = calculator.sequenceHandler()->torsionBasis();

	int idxs[3] = {-1, -1, -1};
	idxs[0] = basis->indexForParameter(t_phi);
	idxs[1] = basis->indexForParameter(t_psi);
	
	basis->setReferenceAngle(idxs[0], 0);
	basis->setReferenceAngle(idxs[1], 0);

	int dims = basis->parameterCount();
	
	grp->recalculate();
	PdbFile pdb("clash_thr.pdb");
	pdb.writeAtoms(grp, "helen");

	std::ofstream file;
	file.open("ramachandran.csv");
	for (float i = 0; i < 360; i++)
	{
		for (float j = 0; j < 360; j++)
		{
			Job job{};
			job.parameters.resize(dims);
			job.parameters[idxs[0]] = i;
			job.parameters[idxs[1]] = j;
			job.requests = JobScoreStructure;
			calculator.submitJob(job);

			Result *result = calculator.acquireResult();
			
			if (result)
			{
				float f = i - 180;
				if (f < 0) f += 360;
				float g = j;
				if (g < 0) g += 360;
				float score = result->score;
//				if (score > 0) score = 0;
				file << f << " " << g << " " << score << std::endl;
				result->destroy();
			}
		}
	}
	file.close();

	calculator.finish();
}
