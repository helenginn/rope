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

#include "RotamerOccupancy.h"
#include "Entity.h"
#include "Instance.h"
#include "Residue.h"
#include "ResidueTorsion.h"
#include "TorsionRef.h"
#include "BondTorsion.h"
#include "AtomGroup.h"
#include "Atom.h"
#include "Progressor.h"
#include "Metadata.h"
#include "matrix_functions.h"

#include <set>
#include <algorithm>
#include <cmath>

RotamerOccupancy::RotamerOccupancy(Entity *entity) : _entity(entity)
{

}

void RotamerOccupancy::calculate(const std::vector<ResidueTorsion> &headers,
                                  Progressor *progress,
                                  std::atomic<bool> *cancelled)
{
	ensureModelsLoaded();

	std::vector<Instance *> instances = _entity->instances();

	for (Instance *instance : instances)
	{
		if (cancelled && cancelled->load())
		{
			break;
		}

		measureInstance(instance, headers);

		if (progress)
		{
			progress->clickTicker();
		}
	}

	unloadModels();

	if (progress)
	{
		progress->finishTicker();
	}
}

void RotamerOccupancy::ensureModelsLoaded()
{
	for (Instance *instance : _entity->instances())
	{
		instance->load();
	}
}

void RotamerOccupancy::unloadModels()
{
	for (Instance *instance : _entity->instances())
	{
		instance->unload();
	}
}

void RotamerOccupancy::measureInstance(Instance *instance,
                                        const std::vector<ResidueTorsion> &headers)
{
	AtomGroup *atoms = instance->currentAtoms();
	if (atoms == nullptr)
	{
		return;
	}

	for (const ResidueTorsion &header : headers)
	{
		ResidueTorsion local = header;
		local.attachToInstance(instance);
		Residue *res = header.master();

		InstanceAngles &instanceAngles = _results[{instance, res}];


		BondTorsion *bt = atoms->findBondTorsion(local.torsion().desc());
		if (bt == nullptr)
		{
			continue;
		}

		std::set<std::string> labels;
		for (int i = 0; i < 4; i++)
		{
			for (const auto &pair : bt->atom(i)->conformerPositions())
			{
				labels.insert(pair.first);
			}
		}

		std::map<std::string, double> &torsionAngles =
		instanceAngles[local.torsion().desc()];

		if (labels.empty())
		{
			glm::vec3 poz[4];
			for (int i = 0; i < 4; i++)
			{
				poz[i] = bt->atom(i)->initialPosition();
			}

			torsionAngles[""] = measure_bond_torsion(poz);
			continue;
		}

		for (const std::string &label : labels)
		{
			glm::vec3 poz[4];
			for (int i = 0; i < 4; i++)
			{
				Atom *a = bt->atom(i);
				const Atom::ConformerInfo &confs = a->conformerPositions();
				auto it = confs.find(label);
				poz[i] = (it != confs.end()) ? it->second.pos.ave
				                              : a->initialPosition();
			}

			if (label.length())
			{
				torsionAngles[label] = measure_bond_torsion(poz);
			}
		}
	}
}

float RotamerOccupancy::altConfOccupancy(Instance *instance, 
                                         Residue *masterResidue,
                                         std::string conf) const
{
	Residue *local = instance->equivalentLocal(masterResidue);
	instance->load();
	AtomGroup *atoms = instance->currentAtoms();

	if (local == nullptr || atoms == nullptr)
	{
		instance->unload();
		return 0.f;
	}

	for (Atom *a : atoms->atomVector())
	{
		if (a->residueId() == local->id() &&
		    a->conformerPositions().count(conf) > 0)
		{
			float val = a->conformerPositions().at(conf).occ;
			instance->unload();
			return val;
		}
	}

	instance->unload();
	return 0.f;
}

bool RotamerOccupancy::hasAltConformers(Residue *masterResidue) const
{
	std::vector<Instance *> instances = _entity->instances();

	for (Instance *instance : instances)
	{
		Residue *local = instance->equivalentLocal(masterResidue);
		AtomGroup *atoms = instance->currentAtoms();

		if (local == nullptr || atoms == nullptr)
		{
			continue;
		}

		for (Atom *a : atoms->atomVector())
		{
			if (a->residueId() == local->id() &&
			    a->conformerPositions().size() > 1)
			{
				return true;
			}
		}
	}

	return false;
}

/** smallest absolute angular separation between two angles in degrees,
 * accounting for wraparound at 360 */
static double angular_diff(double a, double b)
{
	double diff = std::fmod(a - b, 360.0);
	if (diff > 180.0) diff -= 360.0;
	if (diff < -180.0) diff += 360.0;
	return std::fabs(diff);
}

/** this instance's alt-conf labels (or a single "" label if unsplit), each
 * built into a point of chi torsion angles with per-torsion fallback to
 * the shared, unsplit angle where that torsion isn't itself split */
static std::vector<RotamerOccupancy::AltConfPoint>
points_for_instance(Instance *instance,
                    const RotamerOccupancy::InstanceAngles &angles)
{
	std::set<std::string> labels;
	for (const auto &torsionPair : angles)
	{
		for (const auto &labelPair : torsionPair.second)
		{
			if (labelPair.first.length())
			{
				labels.insert(labelPair.first);
			}
		}
	}

	if (labels.empty())
	{
		labels.insert("");
	}

	std::vector<RotamerOccupancy::AltConfPoint> points;

	for (const std::string &label : labels)
	{
		RotamerOccupancy::AltConfPoint pt;
		pt.instance = instance;
		pt.label = label;

		for (const auto &torsionPair : angles)
		{
			const std::map<std::string, double> &byLabel = torsionPair.second;
			auto it = byLabel.find(label);
			if (it == byLabel.end())
			{
				it = byLabel.find("");
			}

			if (it != byLabel.end())
			{
				pt.chiAngles[torsionPair.first] = it->second;
			}
		}

		if (pt.chiAngles.size())
		{
			points.push_back(pt);
		}
	}

	return points;
}

std::vector<RotamerOccupancy::RotamerBucket>
RotamerOccupancy::bucketAltConfs(Residue *r, Metadata *md,
                                 const std::string &header,
                                 double thresholdDegrees) const
{
	std::map<Instance *, InstanceAngles> data = resultsForResidue(r);

	/* walk instances in metadata order - each instance's alt-confs are
	 * then matched 1-to-1 against the buckets' most recently assigned
	 * (i.e. previous instance's) angles, tracking each rotamer along the
	 * ordering rather than against a bucket-wide average. unmatched
	 * points (nothing within thresholdDegrees, or bucket already claimed
	 * this round) start a new bucket. */
	std::vector<Instance *> ordered;
	for (const auto &pair : data)
	{
		ordered.push_back(pair.first);
	}

	std::stable_sort(ordered.begin(), ordered.end(),
	          [md, &header](Instance *a, Instance *b)
	          {
	          	double va = 0, vb = 0;

	          	if (md && header.length())
	          	{
	          		Metadata::KeyValues kva = a->metadata(md);
	          		Metadata::KeyValues kvb = b->metadata(md);

	          		auto ia = kva.find(header);
	          		auto ib = kvb.find(header);

	          		if (ia != kva.end()) va = ia->second.number();
	          		if (ib != kvb.end()) vb = ib->second.number();
	          	}

	          	return va < vb;
	          });

	std::vector<RotamerBucket> buckets;

	for (Instance *instance : ordered)
	{
		std::vector<AltConfPoint> points =
		points_for_instance(instance, data[instance]);

		struct Candidate
		{
			double diff;
			size_t pointIdx;
			size_t bucketIdx;
		};

		std::vector<Candidate> candidates;

		for (size_t p = 0; p < points.size(); p++)
		{
			for (size_t b = 0; b < buckets.size(); b++)
			{
				double maxDiff = 0;
				int compared = 0;

				for (const auto &pair : points[p].chiAngles)
				{
					auto it = buckets[b].average.find(pair.first);
					if (it == buckets[b].average.end())
					{
						continue;
					}

					maxDiff = std::max(maxDiff,
					                   angular_diff(pair.second, it->second));
					compared++;
				}

				if (compared == 0 || maxDiff > thresholdDegrees)
				{
					continue;
				}

				candidates.push_back({maxDiff, p, b});
			}
		}

		std::sort(candidates.begin(), candidates.end(),
		          [](const Candidate &a, const Candidate &b)
		          {
		          	return a.diff < b.diff;
		          });

		std::vector<bool> pointUsed(points.size(), false);
		std::vector<bool> bucketUsed(buckets.size(), false);

		/* 1-to-1: cheapest matches win first, each point and each bucket
		 * can only be claimed once per instance */
		for (const Candidate &c : candidates)
		{
			if (pointUsed[c.pointIdx] || bucketUsed[c.bucketIdx])
			{
				continue;
			}

			pointUsed[c.pointIdx] = true;
			bucketUsed[c.bucketIdx] = true;

			buckets[c.bucketIdx].points.push_back(points[c.pointIdx]);
			buckets[c.bucketIdx].average = points[c.pointIdx].chiAngles;
		}

		for (size_t p = 0; p < points.size(); p++)
		{
			if (pointUsed[p])
			{
				continue;
			}

			RotamerBucket newBucket;
			newBucket.points.push_back(points[p]);
			newBucket.average = points[p].chiAngles;
			buckets.push_back(newBucket);
		}
	}

	std::sort(buckets.begin(), buckets.end(),
	          [](const RotamerBucket &a, const RotamerBucket &b)
	          {
	          	return a.points.size() > b.points.size();
	          });

	return buckets;
}

std::map<Instance *, RotamerOccupancy::InstanceAngles>
RotamerOccupancy::resultsForResidue(Residue *r) const
{
	std::map<Instance *, InstanceAngles> for_res;

	for (const auto &pair : _results)
	{
		if (pair.first.second == r)
		{
			for_res[pair.first.first] = pair.second;
		}
	}
	
	return for_res;
}
