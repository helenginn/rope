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

#define private public // evil but delicious
#include <vagabond/core/Entity.h>
#include <vagabond/core/EntityManager.h>
#include <vagabond/core/Environment.h>
#include <vagabond/c4x/ClusterSVD.h>

namespace tt = boost::test_tools;

BOOST_AUTO_TEST_CASE(molecule_map_molecule)
{
	Environment::env().load("rope.json");
	Entity *ent = &Environment::entityManager()->object(0);
	PositionalGroup group = ent->makePositionalDataGroup();

	Cluster<PositionalGroup> *cx = new ClusterSVD<PositionalGroup>(group);
	cx->cluster();
	
	for (size_t i = 0; i < cx->rows(); i++)
	{
		for (size_t j = 0; j < cx->columns(); j++)
		{
			std::cout << cx->dataGroup()->differenceVector(j)[i] << ", ";
		}
		std::cout << std::endl;
	}
}
