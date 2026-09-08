// vagabond
// Copyright (C) 2026 Helen Ginn
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

#ifndef __vagabond__RotamerOccupancy__
#define __vagabond__RotamerOccupancy__

#include <map>
#include <string>
#include <vector>
#include <atomic>

class Entity;
class Instance;
class Progressor;
class ResidueTorsion;

/** \class RotamerOccupancy measures fresh, per-alt-conformer-label torsion
 * angles for every instance of an Entity, as a first step towards clustering
 * matching rotamers across differently-labelled structures and comparing
 * their occupancies. Separate from OccupanciesView, which predicts
 * occupancy from the energy model rather than comparing observed alt confs. */

class RotamerOccupancy
{
public:
	RotamerOccupancy(Entity *entity);

	/** desc of torsion -> alt-conf label ("" if unsplit) -> angle (degrees) */
	typedef std::map<std::string, std::map<std::string, double>> InstanceAngles;

	/** measures every instance of the entity, optionally reporting progress
	 * and checking for cancellation between instances */
	void calculate(const std::vector<ResidueTorsion> &headers,
	                Progressor *progress = nullptr,
	                std::atomic<bool> *cancelled = nullptr);

	const std::map<Instance *, InstanceAngles> &results() const
	{
		return _results;
	}
private:
	void measureInstance(Instance *instance,
	                      const std::vector<ResidueTorsion> &headers);

	Entity *_entity = nullptr;

	std::map<Instance *, InstanceAngles> _results;
};

#endif
