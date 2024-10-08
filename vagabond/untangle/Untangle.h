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

#ifndef __vagabond__Untangle__
#define __vagabond__Untangle__

#include <set>
#include <map>
#include <list>
#include <mutex>
#include <string>
#include <thread>
#include <functional>
#include "TangledBond.h"
#include <vagabond/utils/OpSet.h>
#include <vagabond/utils/glm_import.h>

class MemoryTangle;
class UntangleView;
class AtomGroup;
class Atom;

class Untangle
{
public:
	Untangle(UntangleView *view, const std::string &filename);

	void setup();

	std::vector<TangledBond *> volatileBonds();

	typedef std::function<void(std::set<Atom *>&, BondLength *)> DownstreamJob;

	void doJobDownstream(Atom *atom, const DownstreamJob &job,
	                     int max_hops = INT_MAX, std::set<Atom *> done = {});
	

	void switchConfs(Atom *a, const std::string &l, 
	                 const std::string &r, bool one_only, 
	                 const std::function<void(Atom *)> &per_job = {});

	void untangle(const std::set<Atom *> &avoid = {});
	
	std::list<TangledBond> &bonds()
	{
		return _bonds;
	}
	
	int firstResidue();
	float biasedScore();
	
	void save(const std::string &filename);
	glm::vec3 positionFor(int resi);
	Atom * atomFor(int resi);
	
	OpSet<TangledBond *> neighbours(TangledBond *first);
	void triggerDisplay();
	void backgroundUntangle(const std::set<Atom *> &avoid);
	
	const std::vector<Atom *> &atoms();
private:
	void loadFile();
	void extract();
	void extractBonds(Atom *const &atom);
	void addTouchedBond(BondLength *bl);

	std::string _filename;
	UntangleView *_view = nullptr;

	AtomGroup *_group = nullptr;
	std::list<TangledBond> _bonds;
	std::vector<TangledBond *> _touched;
	std::map<BondLength *, TangledBond *> _map;
	
	std::thread *_worker = nullptr;
	MemoryTangle *_memory = nullptr;
	std::mutex _memtex;
};

#endif
