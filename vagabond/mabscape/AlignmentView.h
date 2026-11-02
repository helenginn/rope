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
#include <vector>

struct Antigen;
struct Fiducial;
struct DisplayUnit;
class Fiducials;
class Text;
class GlowBall;
class Mesh;

/** an antibody model laid onto the mesh of its antigen by the alignment that
 *  was found for it, with a warning about why that alignment is in doubt. */
class AlignmentView : public Display
{
public:
	AlignmentView(Scene *prev, Antigen &antigen, Fiducial &fiducial,
	              const glm::mat4x4 &transform, const std::string &warning,
	              bool poor = true);
	virtual ~AlignmentView();

	virtual void setup();

	// the other antibodies of this antigen, offered as tickboxes to show
	// each at its first orientation, for comparing positions
	void setComparison(Fiducials &fiducials)
	{
		_fiducials = &fiducials;
	}
private:
	struct Shown
	{
		Fiducial *fid{};
		glm::mat4x4 transform{1.f};
		DisplayUnit *unit{};
		// which of the fiducial's orientations is on show (-1: not one
		// of them, as handed in)
		int orientation{-1};
		// the calculated midpoint of the interface, as a glowing ball
		GlowBall *midpoint{};
	};

	void showFiducial(Shown &shown);
	// destroying: called while the window is already deleting this view, so
	// nothing can be queued for deletion with it (it would wait for itself)
	void hideFiducial(Shown &shown, bool destroying = false);
	void setupTickBoxes();
	// steps the active antibody on to its next (or previous) orientation
	void stepOrientation(int dir);
	void updateOrientationLabel();

	Fiducials *_fiducials{};
	std::vector<Shown> _shown;
	// the antibody which the orientation buttons act on: the one last ticked
	size_t _active{};
	Text *_orientLabel{};

	Antigen &_antigen;
	Fiducial &_fiducial;
	glm::mat4x4 _transform;
	std::string _warning;
	bool _poor{};

	Mesh *_mesh{};
};

#endif
