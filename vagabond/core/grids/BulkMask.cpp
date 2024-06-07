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

#include "../Atom.h"
#include "BulkMask.h"
#include "AtomPosMap.h"
#include "Diffraction.h"
#include "matrix_functions.h"

const int BulkMask::visits[] =
{-2, -2, 0, 
-2, -1, -1, 
-2, -1, 0, 
-2, -1, 1, 
-2, 0, -2, 
-2, 0, -1, 
-2, 0, 0, 
-2, 0, 1, 
-2, 0, 2, 
-2, 1, -1, 
-2, 1, 0, 
-2, 1, 1, 
-2, 2, 0, 
-1, -2, -1, 
-1, -2, 0, 
-1, -2, 1, 
-1, -1, -2, 
-1, -1, -1, 
-1, -1, 0, 
-1, -1, 1, 
-1, -1, 2, 
-1, 0, -2, 
-1, 0, -1, 
-1, 0, 0, 
-1, 0, 1, 
-1, 0, 2, 
-1, 1, -2, 
-1, 1, -1, 
-1, 1, 0, 
-1, 1, 1, 
-1, 1, 2, 
-1, 2, -1, 
-1, 2, 0, 
-1, 2, 1, 
0, -2, -2, 
0, -2, -1, 
0, -2, 0, 
0, -2, 1, 
0, -2, 2, 
0, -1, -2, 
0, -1, -1, 
0, -1, 0, 
0, -1, 1, 
0, -1, 2, 
0, 0, -2, 
0, 0, -1, 
0, 0, 0, 
0, 0, 1, 
0, 0, 2, 
0, 1, -2, 
0, 1, -1, 
0, 1, 0, 
0, 1, 1, 
0, 1, 2, 
0, 2, -2, 
0, 2, -1, 
0, 2, 0, 
0, 2, 1, 
0, 2, 2, 
1, -2, -1, 
1, -2, 0, 
1, -2, 1, 
1, -1, -2, 
1, -1, -1, 
1, -1, 0, 
1, -1, 1, 
1, -1, 2, 
1, 0, -2, 
1, 0, -1, 
1, 0, 0, 
1, 0, 1, 
1, 0, 2, 
1, 1, -2, 
1, 1, -1, 
1, 1, 0, 
1, 1, 1, 
1, 1, 2, 
1, 2, -1, 
1, 2, 0, 
1, 2, 1, 
2, -2, 0, 
2, -1, -1, 
2, -1, 0, 
2, -1, 1, 
2, 0, -2, 
2, 0, -1, 
2, 0, 0, 
2, 0, 1, 
2, 0, 2, 
2, 1, -1, 
2, 1, 0, 
2, 1, 1, 
2, 2, 0};


BulkMask::BulkMask(Diffraction *blueprint, float spacing)
: CubicGrid<Masker>(0, 0, 0)
{
	std::array<double, 6> cell = blueprint->list()->unitCell();

	int nx = ceil(cell[0] / spacing);
	int ny = ceil(cell[1] / spacing);
	int nz = ceil(cell[2] / spacing);

	// will overhang, but we can ignore it until the end if we work in 
	// inverted space
	this->TransformedGrid<Masker>::setDimensions(nx, ny, nz, false); 

	this->setRealDim(spacing);
}

void BulkMask::addPosList(Atom *atom, const WithPos &wp)
{
	float rad = atom->elementSymbol() == "H" ? 1.2 : 2.4; // fix

	luint i = 0;
	for (const glm::vec3 &p : wp.samples)
	{
		addSphere(i, p, rad);
		i++;
	}
}

void BulkMask::addSphere(luint bit, glm::vec3 pos, const float &radius)
{
	int voxRadius = ceil(radius / realDim());
	real2Voxel(pos);
//	glm::vec3 remainder = pos;
	
	int idx = 0;
	while (bit > 64)
	{
		bit -= 64;
		idx = 1;
	}

	for (int z = -voxRadius; z <= voxRadius; z++)
	{
		for (int y = -voxRadius; y <= voxRadius; y++)
		{
			for (int x = -voxRadius; x <= voxRadius; x++)
			{
				glm::vec3 xyz = glm::vec3(x, y, z);
				
				/* error: will be out by the non-integer remainder in pos */
				float ll = glm::dot(xyz, xyz);
				if (ll > voxRadius * voxRadius)
				{
					continue;
				}
				
				xyz += pos;
				luint add = (1 << bit);

				Masker &m = element(xyz.x, xyz.y, xyz.z);
				m.mask[idx] |= add;
			}
		}
	}
}

void BulkMask::contract()
{
	auto print_byte = [](luint &val)
	{
		std::cout << "0b" << std::endl;
		for (int i = 0; i < 64; i++)
		{
			std::cout << ((val << i & 1) ? "1" : "0");
		}
		std::cout << std::endl;
	};

	auto transfer_zeros = [this, print_byte](const long int &idx, luint *first)
	{
		for (int i = 0; i < 2; i++)
		{
			luint &sum = first[i];
			luint &contribution = _data[idx].mask[i];
			sum &= contribution;
		}
	};

	auto contracted_all_ones = [](Masker &masked)
	{
		for (int i = 0; i < 2; i++)
		{
			masked.contracted[i] = -1;
		}
	};

	auto transfer_contract_to_mask = [this](Masker &masked)
	{
		for (int i = 0; i < 2; i++)
		{
			masked.mask[i] = masked.contracted[i];
		}
	};

	auto transfer_zeros_from_neighbours = [this, transfer_zeros]
	(int i, int j, int k)
	{
		float num = sizeof(visits) / sizeof(visits[0]);
		long int idx = this->index(i, j, k);
		for (int n = 0; n < num; n += 3)
		{
			long int vIdx = this->index(i + visits[n], 
			                            j + visits[n+1],
			                            k + visits[n+2]);
			transfer_zeros(vIdx, _data[idx].contracted);
		}
	};

	do_op_on_each_element(contracted_all_ones);
	do_op_on_basic_index(transfer_zeros_from_neighbours);
	do_op_on_each_element(transfer_contract_to_mask);
}
