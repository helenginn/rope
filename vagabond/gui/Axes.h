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

#ifndef __vagabond__Axes__
#define __vagabond__Axes__

#include <vagabond/gui/elements/IndexResponder.h>
#include <vagabond/c4x/Cluster.h>

class Molecule;
class MetadataGroup;

class Axes : public IndexResponder
{
public:
	Axes(Cluster<MetadataGroup> *group, Molecule *m);

	virtual void interacted(int idx, bool hover);
	virtual void reindex();
	virtual void click();
	virtual bool mouseOver();
	virtual void unmouseOver();

	virtual size_t requestedIndices();
private:
	void prepareAxes();

	Cluster<MetadataGroup> *_cluster = nullptr;
	Molecule *_molecule = nullptr;

	std::vector<glm::vec3> _dirs;
};

#endif
