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

#ifndef __vagabond__PlaneView__
#define __vagabond__PlaneView__

#include <vagabond/gui/elements/Renderable.h>
#include <vagabond/core/Plane.h>

class Molecule;
class PlaneGrid;
class MetadataGroup;

template <class DG>
class Cluster;

class PlaneView : public Renderable, public Responder<Plane>
{
public:
	PlaneView(Plane *p);
	PlaneView(Cluster<MetadataGroup> *cluster, Molecule *mol);
	virtual ~PlaneView();
	
	Plane *plane()
	{
		return _plane;
	}
	
	void setPlanes(bool planes[3])
	{
		for (size_t i = 0; i < 3; i++)
		{
			_planes[i] = planes[i];
		}
	}
	
	bool *planes()
	{
		return _planes;
	}

	void populate();
	void recolour();
	virtual void respond();
	virtual void render(SnowGL *gl);
private:
	void makeGrid();

	Plane *_plane = nullptr;
	PlaneGrid *_pg = nullptr;

	bool _planes[3];
};

#endif
