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

#include <fstream>
#include "Separation.h"
#include "BondSequence.h"
#include "CompareAtoms.h"

using Matrix = Eigen::MatrixXi;

void wipe(Matrix &mat)
{
	for (int i = 0; i < mat.rows(); i++)
	{
		for (int j = 0; j < mat.cols(); j++)
		{
			mat(i, j) = -1;
		}
	}
}

void Separation::prepare(const std::vector<Atom *> &atoms)
{
	std::vector<std::vector<Atom *>> set;

	std::vector<Atom *> wip;
	for (int i = 0; i < atoms.size(); i++)
	{
		Atom *a = atoms[i];
		if (!a && wip.size())
		{
			set.push_back(wip);
			wip.clear();
		}
		else
		{
			wip.push_back(a);
		}
	}

	if (wip.size())
	{
		set.push_back(wip);
	}
	
	_atoms = SortedVector(atoms);
	int n = _atoms.size();
	_matrix = Matrix(n, n);

	wipe(_matrix);
	
	std::cout << "Atoms: ";
	for (std::vector<Atom *> &wip : set)
	{
		std::cout << wip.size() << ", ";
		prepareSegment(wip);
	}
	std::cout << std::endl;
	
	std::ofstream file;
	file.open("check_matrix.csv");
	file << _matrix << std::endl;
	file.close();
}

void Separation::prepareSegment(const std::vector<Atom *> &atoms)
{
	OpSet<Atom *> included;
	OpSet<Atom *> leads;

	for (int i = 0; i < atoms.size(); i++)
	{
		Atom *a = atoms[i];
		if (!a) continue;
		int a_idx = _atoms.index_of(a);
		if (a_idx >= 0)
		{
			leads.insert(a);
			included.insert(a);
			break;
		}
	}

	Separation &me = *this;

	while (leads.size())
	{
		OpSet<Atom *> next_leads;
		for (Atom *const a : leads)
		{
			for (int j = 0; j < a->bondLengthCount(); j++)
			{
				Atom *b = a->connectedAtom(j);
				if (included.count(b))
				{
					continue;
				}
				if (a->atomName() == "SG" && b->atomName() == "SG")
				{
					continue;
				}

				int b_idx = _atoms.index_of(b);
				if (b_idx < 0)
				{
					continue;
				}

				for (Atom *const prev : included)
				{
					if (_atoms.index_of(prev) < 0) { continue; }

					int prev_to_a = me(prev, a) < 0 ? 0 : me(prev, a);
					int increment = prev_to_a + 1;
					int current = me(prev, b);
					
					if (current < 0 || current > increment)
					{
						me(prev, b) = increment;
						me(b, prev) = increment;
					}
				}

				included.insert(b);
				next_leads.insert(b);
			}
		}

		leads = next_leads;
	}
}

Separation::Separation(BondSequence *const &sequence)
{
	std::vector<Atom *> atoms(sequence->blockCount());

	int n = 0;
	for (const AtomBlock &block : sequence->blocks())
	{
		atoms[n] = block.atom;
		n++;
	}
	
	prepare(atoms);
}

int Separation::separationBetween(Atom *const &a, Atom *const &b)
{
	int i = _atoms.index_of(a);
	int j = _atoms.index_of(b);
	if (i < 0 || j < 0)
	{
		return -1;
	}

	return _matrix(i, j);
}

Separation::SortedVector::SortedVector(std::vector<Atom *> atoms) //:
{
	_atoms = atoms;
	for (int i = 0; i < atoms.size(); i++)
	{
		_map[atoms[i]] = i;
	}
}

int Separation::SortedVector::index_of(Atom *const &ptr)
{
	if (_map.count(ptr) == 0)
	{
		return -1;
	}
	else
	{
		return _map.at(ptr);
	}
}

