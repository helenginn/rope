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

#include "Metadata.h"
#include "PathGroup.h"
#include "PathData.h"
#include "Path.h"

PathData *PathGroup::preparePathData()
{
	if (size() == 0) return nullptr;
	
	Path *const &ref = at(0);

	size_t size = ref->motionCount() * 2;

	PathData *pd = new PathData(size);
	
	std::vector<ResidueTorsion> headers;
	for (int i = 0; i < ref->motionCount(); i++)
	{
		headers.push_back(ref->motions().rt(i));
	}

	pd->addHeaders(headers);
	
	for (Path *const &path : *this)
	{
		path->addToData(pd);
	}

	return pd;
}

Metadata *PathGroup::prepareMetadata()
{
	Metadata *metadata = new Metadata();
	for (Path *const &path : *this)
	{
		if (!path->hasScores())
		{
			PlausibleRoute *pr = path->toRoute();
			pr->setup();
			pr->refreshScores();
			path->cleanupRoute();
		}
		
		float clash = path->clashScore();
		float activation_energy = path->activationEnergy();

		std::string id = path->id();
		Metadata::KeyValues kv = 
		{{"clash", Value(clash)}, 
		{"activation energy", Value(activation_energy)}, 
		{"momentum", Value(path->momentumScore())}, 
		{"instance", id}};

		metadata->addKeyValues(kv, true);
	}

	return metadata;
}
