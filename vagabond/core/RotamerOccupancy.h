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
class Metadata;
class Progressor;
class Residue;
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
	typedef std::pair<Instance *, Residue *> InstRes;

	/** measures every instance of the entity, optionally reporting progress
	 * and checking for cancellation between instances */
	void calculate(const std::vector<ResidueTorsion> &headers,
	                Progressor *progress = nullptr,
	                std::atomic<bool> *cancelled = nullptr);

	/** Atom::conformerPositions() is only populated once a model has been
	 * loaded from disk - call this once before a batch of calls that need
	 * real atom data (e.g. hasAltConformers() over every residue), and
	 * unloadModels() once afterwards. calculate() does this internally. */
	void ensureModelsLoaded();
	void unloadModels();

	std::map<Instance *, InstanceAngles> resultsForResidue(Residue *r) const;

	const std::map<InstRes, InstanceAngles> &results() const
	{
		return _results;
	}

	/** one alt-conf label of one instance, as a point in chi-torsion space */
	struct AltConfPoint
	{
		Instance *instance = nullptr;
		std::string label;
		/** torsion desc -> angle (degrees) */
		std::map<std::string, double> chiAngles;
	};

	struct RotamerBucket
	{
		std::vector<AltConfPoint> points;
		/** chi torsions of the most recently assigned point (i.e. the
		 * previous instance, in metadata order, tracked into this bucket) */
		std::map<std::string, double> average;
	};

	/** Walks every instance for the given residue in ascending order of
	 * its value for the given metadata header (instances lacking a value,
	 * or md/header left empty, sort as if their value were 0), and matches
	 * each instance's alt-conf labels 1-to-1 against the buckets' most
	 * recently tracked angles - so a bucket represents one rotamer
	 * followed along the ordering, not an overall average. Unmatched
	 * points (nothing within thresholdDegrees on every shared torsion, or
	 * the closest bucket already claimed by a cheaper match this round)
	 * start a new bucket. Buckets are returned in descending order of
	 * size. */
	std::vector<RotamerBucket> bucketAltConfs(Residue *r, Metadata *md,
	                                          const std::string &header,
	                                          double thresholdDegrees = 45.0) const;

	/** true if any instance has more than one alt-conf label on any atom
	 * of the given master residue - i.e. there's something to compare */
	bool hasAltConformers(Residue *masterResidue) const;

	float altConfOccupancy(Instance *inst, Residue *masterResidue,
	                       std::string conf) const;
private:
	void measureInstance(Instance *instance,
	                      const std::vector<ResidueTorsion> &headers);

	Entity *_entity = nullptr;

	std::map<InstRes, InstanceAngles> _results;
};

#endif
