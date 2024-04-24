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

#include "Separation.h"
#include "BondSequence.h"
#include "CompareAtoms.h"

using Matrix = Eigen::MatrixXi;

void fill_distances(Matrix &m)
{
	int count = m.rows();

	for (size_t k = 0; k < count; k++)
	{
		for (size_t i = 0; i < count - 1; i++)
		{
			for (size_t j = i + 1; j < count; j++)
			{
				int ij = m(i, j);
				int ik = m(i, k);
				int kj = m(k, j);

				int sum = ik + kj;
				if (ik >= 0 && kj >= 0)
				{
					if (ij < 0)
					{
						m(i, j) = sum;
						m(j, i) = sum;
					}
					else if (ij > sum)
					{
						m(i, j) = sum;
						m(j, i) = sum;
					}
				}
			}
		}
	}
}

void wipe(Matrix &mat)
{
	for (int i = 0; i < mat.rows(); i++)
	{
		for (int j = 0; j < mat.cols(); j++)
		{
			mat(i, j) = (i == j) ? 0 : -1;
		}
	}
}

void Separation::prepare(const std::vector<Atom *> &atoms)
{
	_atoms = SortedVector(atoms);
	int n = _atoms.size();
	_matrix = Matrix(n, n);

	wipe(_matrix);
	
	for (int i = 0; i < _atoms.size(); i++)
	{
		Atom *a = _atoms._atoms[i];
		if (!a) continue;

		for (int j = 0; j < a->bondLengthCount(); j++)
		{
			Atom *b = a->connectedAtom(j);
			int b_idx = _atoms.index_of(b);
			if (b_idx >= 0)
			{
				_matrix(i, b_idx) = 1;
			}

		}
	}
	
	fill_distances(_matrix);
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
//_compare(atom_ptr_compare_function())
{
//	_compare = atom_ptr_compare_function();
	_atoms = atoms;

//	std::sort(_atoms.begin(), _atoms.end(), _compare);
}

int Separation::SortedVector::index_of(Atom *const &ptr)
{
	auto it = std::find(_atoms.begin(), _atoms.end(), ptr);
	if (it == _atoms.end()) return -1;
	return (&*it - &_atoms[0]);

	/*
	size_t lower = 0;
	size_t upper = _atoms.size();

	if (_compare(ptr, _atoms[lower])) { return -1; }
	if (_compare(_atoms[upper], ptr)) { return -1; }
	if (_atoms[lower] == ptr) { return lower; }

	while (true)
	{
		size_t range = (upper + lower);
		size_t middle = (range % 2 == 0) ? (range / 2) : (range + 1) / 2;
		if (_atoms[middle] == ptr)
		{
			return middle;
		}

		(_compare(ptr, _atoms[middle]) ? upper : lower) = middle;
	}
	
	return -1;
	*/
}
