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

#ifndef __vagabond__Loopy__
#define __vagabond__Loopy__

#include <vector>
#include <string>
#include <thread>
#include "Responder.h"
#include "StructureModification.h"
#include "ParamSet.h"
#include "Conformer.h"
#include "Atom.h"
#include "ListConformers.h"
#include "ResidueId.h"

class Instance;
class Diffraction;
class ArbitraryMap;
class LoopCorrelation;
class FilterConformer;
class FilterSelfClash;
class FilterCrystalContact;

namespace gemmi
{
	struct Structure;
}

struct Loop
{
	int start;
	int end;
	std::string seq;
	int offset;
	Instance *instance;
	
	int length() const
	{
		return end - start;
	}

	int instance_start() const
	{
		return offset;
	}

	int instance_end() const
	{
		return offset + (end - start);
	}

	bool rightOfLoop(const ResidueId &id) const
	{
		return (id.as_num() > instance_end());
	}

	bool idInLoop(const ResidueId &id) const
	{
		return (id.as_num() > instance_start() &&
		        id.as_num() <= instance_end());
	}

	bool idNeedsClashCheck(const ResidueId &id) const
	{
		return (id.as_num() > instance_start() - 1 &&
		        id.as_num() <= instance_end() + 1);
	}

	auto mainChainInLoop()
	{
		return [this](Atom *atom)
		{
			// not part of loop
			if (!atom || !idInLoop(atom->residueId()))
			{
				return false;
			}

			if (!atom->isMainChain())
			{
				return false;
			}

			return true;
		};
	}
};

class RoughLoop;
class Polymer;
class SequenceComparison;
class ArbitraryMap;

class Loopy : public HasResponder<Responder<Loopy>>, public StructureModification
{
	friend RoughLoop;
	friend LoopCorrelation;
	friend FilterConformer;
	friend FilterSelfClash;
	friend FilterCrystalContact;
public:
	Loopy(Polymer *instance);
	~Loopy();
	
	size_t size()
	{
		return _loops.size();
	}
	
	const Loop &loop() const
	{
		return _loops[0];
	}
	
	// add atom positions to Atom "loop" positions
	void operator()(int i);

private:
	Polymer *_instance = nullptr;

	/* finding loops */
	void findLoops();
	void getLoop(SequenceComparison *sc, int start, int end);
	
	/* preparing space to refine */
	void processLoop(Loop &loop);
	void prepareLoop(const Loop &loop);
	void prepareMaps(bool all_atoms = true);
	void prepareResources();
	void grabNewParameters();

	/* refinement jobs */
	float roughLoop();
	ListConformers generateRoughs(int n);

	/* map calculations */
	void loadDensityMap();
	void fCalcMap();

	/* writing out to PDB files */
	void copyPositions(const std::string &tag);
	void writeOutPositions(const std::string &tag);
	void writeOutPositions(gemmi::Structure &st, const std::string &tag, int i);
	
	void progressReport();

	std::vector<Loop> _loops;

	void randomise(Conformer &active);
	
	ListConformers _generated, _nonclash, _noncontact;
	ArbitraryMap *_fcMap = nullptr;

	Diffraction *_data = nullptr;
	ArbitraryMap *_map = nullptr;

	Conformer _active;

	int _threads = 1;
	int _ticket = 0;
	int _count = 0;
	float _rmsdLimit = 0.1;

};

#endif
