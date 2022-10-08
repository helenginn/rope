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

#ifndef __vagabond__Path__
#define __vagabond__Path__

#include "PlausibleRoute.h"
#include "Molecule.h"

class Path
{
public:
	Path(PlausibleRoute *pr);

	void getTorsionRef(int idx);

	friend void to_json(json &j, const Path &value);
	friend void from_json(const json &j, Path &value);
	
	PlausibleRoute *toRoute();
	void housekeeping();
private:
	std::string _startMolecule;
	std::string _model_id;
	std::string _endMolecule;
	Molecule *_molecule = nullptr;
	Model *_model = nullptr;
	Molecule *_end = nullptr;

	std::map<int, PlausibleRoute::WayPoints> _wayPoints;
	std::vector<bool> _flips;

	PlausibleRoute::Point _destination;
	PlausibleRoute::InterpolationType _type = PlausibleRoute::Polynomial;
	
	PlausibleRoute *_route = nullptr;
	
	std::vector<ResidueId> _residueIds;
	std::vector<TorsionRef> _torsionRefs;
};

/* waypoint */
inline void to_json(json &j, const PlausibleRoute::WayPoint &value)
{
	j["p"] = value.progress;
	j["f"] = value.fraction;
}

/* waypoint */
inline void from_json(const json &j, PlausibleRoute::WayPoint &value)
{
    value.progress = j.at("p");
    value.fraction = j.at("f");
}

/* path */
inline void to_json(json &j, const Path &value)
{
	j["residues"] = value._residueIds;
	j["torsions"] = value._torsionRefs;
	j["flips"] = value._flips;
	j["waypoints"] = value._wayPoints;
}

/* path */
inline void from_json(const json &j, Path &value)
{
	value._startMolecule = j.at("start");
	value._endMolecule = j.at("end");
	value._model_id = j.at("model");

    std::vector<ResidueId> residues = j.at("residues");
    value._residueIds = residues;
    std::vector<TorsionRef> refs = j.at("torsions");
    value._torsionRefs = refs;

    std::vector<bool> flips = j.at("flips");
    value._flips = flips;

    std::map<int, PlausibleRoute::WayPoints> wps = j.at("waypoints");
    value._wayPoints = wps;
	
	value.housekeeping();
}

#endif
