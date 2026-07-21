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

#ifndef __vagabond__Energy__
#define __vagabond__Energy__

#include "hnet.h"
#include "Probe.h"
#include <map>

namespace hnet
{
	typedef std::function<float()> GetEnergy;
	typedef std::function<GetEnergy()> EnergyWrapper;

class Energy
{
public:
	Energy();

	enum Source
	{
		Torsion,
		Acceptor,
		Distance,
		Angle,
		Bulk,
		Unknown,
	};

	void alter_amplification(const Source &src, float amp)
	{
		_amplifiers[src] = amp;
	}
	
	void alter_source(const Source &src, bool on)
	{
		_sources[src] = on;
	}

	EnergyWrapper energy_wrapper_for_covalent(BondProbe &bp);

	EnergyWrapper energy_wrapper_for_half_hbond(HydrogenProbe *probe, 
	                                            BondProbe &bp, glm::vec3 pos);

	EnergyWrapper energy_wrapper_for_liberated_bulk(AtomProbe &bulk);

	EnergyWrapper energy_wrapper_for_hbond_angle(HydrogenProbe *probe,
	                                             BondProbe &left,
	                                             BondProbe &right,
	                                             AtomProbe &lAtom,
	                                             AtomProbe &rAtom);
private:
	
	typedef std::pair<GetEnergy, Source> SourcedEnergy;

	EnergyWrapper modulate(const std::vector<SourcedEnergy> &sources);

	std::map<Source, bool> _sources;
	std::map<Source, float> _amplifiers;
};
};

#endif
