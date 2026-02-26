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
	std::map<BlockRegion, std::vector<Atom *>> set;

	std::vector<Atom *> wip;
	int start = 0; int end = 0;
	for (int i = 0; i < atoms.size(); i++)
	{
		Atom *a = atoms[i];
		if (!a && wip.size())
		{
			BlockRegion block(start, end);
			set[block] = wip;
			wip.clear();
			start = i + 1; end = i + 1;
		}
		else
		{
			_indices[a] = i;
			wip.push_back(a);
			end++;
		}
	}

	if (wip.size())
	{
		set[{start, end}] = wip;
	}
	
	for (auto it = set.begin(); it != set.end(); it++)
	{
		BlockRegion start_end = it->first;
		std::vector<Atom *> &atomRun = it->second;
		_atoms[start_end] = SortedVector(atomRun);
		int n = atomRun.size();
		_matrices[start_end] = Matrix(n, n);

		wipe(_matrices[start_end]);
	}

	std::cout << "Atoms: ";
	for (auto it = _atoms.begin(); it != _atoms.end(); it++)
	{
		SortedVector &atomRun = it->second;
		BlockRegion pair = it->first;
		for (Atom *atom : atomRun._atoms)
		{
			_grouping[atom] = pair;
		}
		
		_starts.insert(pair.first);
		_idxGroups[pair.first] = pair;

		std::cout << atomRun.size() << ", ";
		prepareSegment(atomRun);
	}
	std::cout << std::endl;
}

void Separation::prepareSegment(const SortedVector &atoms)
{
	OpSet<Atom *> included;
	OpSet<Atom *> leads;

	for (int i = 0; i < atoms.size(); i++)
	{
		Atom *a = atoms._atoms[i];
		if (!a) continue;
		int a_idx = atoms.index_of(a);
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

				int b_idx = atoms.index_of(b);
				if (b_idx < 0)
				{
					continue;
				}

				for (Atom *const prev : included)
				{
					if (atoms.index_of(prev) < 0) { continue; }

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

Eigen::MatrixXi::Scalar &Separation::operator()(Atom *const &a, Atom *const &b)
{
	BlockRegion aGroup = _grouping[a];
	BlockRegion bGroup = _grouping[b];
	
	if (aGroup != bGroup)
	{
		return _fail;
	}

	// rewrite for speed
	int i = _atoms[aGroup].index_of(a);
	int j = _atoms[bGroup].index_of(b);

	if (i < 0 || j < 0)
	{
		return _fail;
	}

	return _matrices[aGroup](i, j);
}

Separation::BlockRegion &Separation::group_for_idx(int i)
{
	GroupStarts::iterator it = _starts.lower_bound(i);
	it--;
	BlockRegion &group = _idxGroups[*it];
	return group;
}

bool Separation::partOfSameMolecule(int m, int n)
{
	BlockRegion &aGroup = _idxGroups[m];
	bool result = (n <= aGroup.second && n >= aGroup.first);
	return result;
}

int Separation::separationBetween(Atom *const &a, Atom *const &b)
{
	int val = (*this)(a, b);
	return val;
}


int Separation::separationBetween(int i, int j)
{
	BlockRegion group = group_for_idx(i);

	if (j > group.second || j < group.first)
	{
		return -1;
	}
	
	return _matrices[group](i - group.first, j - group.first);
}

Separation::SortedVector::SortedVector(std::vector<Atom *> atoms) //:
{
	_atoms = atoms;
	for (int i = 0; i < atoms.size(); i++)
	{
		_map[atoms[i]] = i;
	}
}

int Separation::SortedVector::index_of(Atom *const &ptr) const
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

