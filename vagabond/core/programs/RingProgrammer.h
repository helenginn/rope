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

#ifndef __vagabond__RingProgrammer__
#define __vagabond__RingProgrammer__

#include <mutex>
#include "programs/Cyclic.h"
#include "programs/ExitGroup.h"
#include "AtomBlock.h"

class Atom;
class TorsionBasis;
struct AtomGraph;
class RingProgram;

class RingProgrammer
{
public:
	RingProgrammer();
	
	static std::vector<RingProgrammer *> *allProgrammers();
	std::string paramSpec(int i, float *def);
	
	void updateValue(AtomGroup *group, HyperValue *hv, int i);
	
	size_t paramSpecCount() const
	{
		return _paramSpecs.size();
	}
	
	const std::string &pinnedAtom() const
	{
		return _pinnedAtom;
	}

	RingProgram *program()
	{
		return _program;
	}
	
	void reset();
	
	const Cyclic &cyclic() const
	{
		return _cyclic;
	}

	const std::string &code() const
	{
		return _code;
	}
	
	const int &triggerIndex() const
	{
		return _triggerIndex;
	}
	
	const std::vector<std::string> &ringMembers() const
	{
		return _cyclic.atomNames();
	}
	
	/** call while converting atoms to blocks in order to substantiate
	 * a program if the conditions for a program are met
	 * @returns whether programmer should now be duplicated */
	bool registerAtom(AtomGraph *ag, int idx);
	
	/* ask after each atom registry if the program entry conditions
	 * have been met */
	bool isProgramTriggered();
	
	/* once a program has been triggered, ask if exit conditions have
	 * been met in order to conclude the program override */
	bool areExitConditionsMet()
	{
		return _complete;
	}
	
	std::string status();

	/* call in order to generate a RingProgram which will fill in the
	 * corresponding atom blocks */
	void makeProgram(std::vector<AtomBlock> &blocks, int prog_num,
	                 TorsionBasis *basis);

	friend void to_json(json &j, const RingProgrammer &value);
	friend void from_json(const json &j, RingProgrammer &value);
protected:
	
private:
	bool registerAtom(Atom *a, int idx);
	void wipeFlagsExcept(int idx);
	bool proofSolution(int grp_idx);
	void fixProgramIndices(std::vector<AtomBlock> &blocks,
	                       int prog_num);

	void findGroupLocations(int grp_idx);
	void grabAtomLocation(Atom *atom, int idx);
	bool groupsComplete();
	void correctIndexOffset();
	bool registerWithGroup(rope::ExitGroup &group, Atom *atom, int idx);

	// this group are assigned from day one
	std::vector<rope::ExitGroup> _groups;
	Cyclic _cyclic;
	std::string _code;
	std::string _pinnedAtom;
	
	struct ParamSpec
	{
		std::string param_name; /** name of this parameter */
		float default_value; /** default value of this parameter */
		std::string access_atom; /** atom with mirroring torsion angle */
		std::string mirror_param; /** name of parameter with estimation value */
	};

	friend void from_json(const json &j, RingProgrammer::ParamSpec &value);
	friend void to_json(json &j, const RingProgrammer::ParamSpec &value);

	std::vector<ParamSpec> _paramSpecs;
	
	std::map<std::string, int> _atomLocs;
	std::map<Atom *, int> _atomPtrLocs;
	std::map<std::string, int> _branchLocs;
	std::map<int, AtomGraph *> _idx2Graph;
	std::map<AtomGraph *, int> _graph2Idx;
	std::map<std::string, std::string> _grandparents;

	std::string _cyclicFile;

	RingProgram *_program = nullptr;

	int _entrance = -1;
	// duplicated is set to TRUE when RingProgrammer spawns a second copy
	bool _duplicated = false;
	ResidueId _activeId{};
	bool _complete = false;
	
	int _triggerIndex = -1;

	static std::mutex _mutex;
	static std::vector<RingProgrammer *> _rammers;
};

inline void to_json(json &j, const RingProgrammer::ParamSpec &value)
{
	j["param_name"] = value.param_name;
	j["default_value"] = value.default_value;
	j["access_atom"] = value.access_atom;
	j["mirror_param"] = value.mirror_param;
}

inline void from_json(const json &j, RingProgrammer::ParamSpec &value)
{
	value.param_name = j["param_name"];
	value.default_value = j["default_value"];
	value.access_atom = j["access_atom"];
	value.mirror_param = j["mirror_param"];
}

inline void to_json(json &j, const RingProgrammer &value)
{
	j["cyclic"] = value._cyclic;

	j["groups"] = value._groups;

	j["code"] = value._code;
	j["pinned"] = value._pinnedAtom;
	j["param_specs"] = value._paramSpecs;
}

inline void from_json(const json &j, RingProgrammer &value)
{
	value._cyclic = j["cyclic"];

	if (j.count("groups"))
	{
		std::vector<rope::ExitGroup> groups = j["groups"];
		value._groups = groups;
	}

	if (j.count("code"))
	{
		value._code = j["code"];
	}

	if (j.count("pinned"))
	{
		value._pinnedAtom = j["pinned"];
	}

	if (j.count("param_specs"))
	{
		std::vector<RingProgrammer::ParamSpec> paramSpecs = j["param_specs"];
		value._paramSpecs = paramSpecs;
	}
}


#endif
