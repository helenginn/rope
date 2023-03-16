// vagabond
// Copyright (C) 2019 Helen Ginn
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

#ifndef __vagabond__AlignmentTool__
#define __vagabond__AlignmentTool__

#include "../utils/glm_import.h"

class AtomGroup;

struct Result;
class Atom;

class AlignmentTool
{
public:
	AlignmentTool(AtomGroup *group);

	void run();
	void run(Atom *anchor);
private:
	int calculateExtension(Atom *anchor);
	Result *resultForAnchor(Atom *anchor);
	glm::mat4x4 superposition(Result *result);

	AtomGroup *_group;
};

#endif
