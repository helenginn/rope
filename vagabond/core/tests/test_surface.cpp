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

#define protected public // evil but delicious
#define private public // evil but delicious

#include <vagabond/core/AtomGroup.h>
#include <vagabond/core/BondCalculator.h>

namespace tt = boost::test_tools;

BOOST_AUTO_TEST_CASE(oxygen_atom_has_surface_area)
{
	// oxygen atom Van der Waals radius is 1.52 Ang according to Google.
	// 4/3 * pi * r^3 is therefore 14.7 Ang.

	Atom a;
	a.setElementSymbol("O");
	
	AtomGroup grp;
	grp += &a;
	
	BondCalculator calc;
	calc.setPipelineType(BondCalculator::PipelineSolventSurfaceArea);
	calc.addAnchorExtension(&a);
	
	calc.setup();
	calc.start();
	
	Job job{};
	job.requests = static_cast<JobType>(JobSolventSurfaceArea);

	calc.submitJob(job);

	Result *r = calc.acquireResult();
	
	float area = r->surface_area;
	BOOST_TEST(area == 14.7102, tt::tolerance(1e-2));
}
