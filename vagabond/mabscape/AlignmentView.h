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

#ifndef __vagabond__AlignmentView__
#define __vagabond__AlignmentView__

#include <vagabond/gui/Display.h>
#include <string>

struct Antigen;
struct Fiducial;
struct DisplayUnit;
class Mesh;

/** an antibody model laid onto the mesh of its antigen by the alignment that
 *  was found for it, with a warning about why that alignment is in doubt. */
class AlignmentView : public Display
{
public:
	AlignmentView(Scene *prev, Antigen &antigen, Fiducial &fiducial,
	              const glm::mat4x4 &transform, const std::string &warning);
	virtual ~AlignmentView();

	virtual void setup();
private:
	Antigen &_antigen;
	Fiducial &_fiducial;
	glm::mat4x4 _transform;
	std::string _warning;

	Mesh *_mesh{};
	DisplayUnit *_unit{};
};

#endif
