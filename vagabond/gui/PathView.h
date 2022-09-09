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

#ifndef __vagabond__PathView__
#define __vagabond__PathView__

#include <thread>
#include <vagabond/gui/elements/Renderable.h>
#include <vagabond/core/PathFinder.h>

class Molecule;
class MetadataGroup;

template <class DG>
class Cluster;

class PathView : public Renderable, public Responder<PathFinder>
{
public:
	PathView(Cluster<MetadataGroup> *cluster, Molecule *mol);
	virtual ~PathView();

	PathFinder *pathFinder()
	{
		return _pathFinder;
	}
	
	void setTarget(Molecule *m)
	{
		_pathFinder->setTarget(m);
	}

	void populate();
	void addNewPoints();
	virtual void respond();

	void start();
private:
	PathFinder *_pathFinder = nullptr;
	Cluster<MetadataGroup> *_cluster = nullptr;
	
	std::thread *_worker = nullptr;
	Molecule *_molecule = nullptr;

	float _first = 0;
};

#endif
