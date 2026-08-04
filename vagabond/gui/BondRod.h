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

#ifndef __vagabond__BondRod__
#define __vagabond__BondRod__

#include <vagabond/gui/elements/Image.h>

/** Shared fixed-4-vertex, in-place-overwrite bond-line quad, used by both
 *  ProbeBond (ProtonNetworkView's interactive H-bond editor) and
 *  HBondDiagram's read-only bond lines - never reallocates its vertex/
 *  index arrays, so it is safe to call every tick from PositionShifter's
 *  background thread (unlike ThickLine::reset()+addPoint(), which
 *  reallocates on every call - see ClusterPlot's history for the crash
 *  that caused). Rendered via assets/shaders/axes.vsh/.fsh, which expects
 *  tex[0] = +-0.5 (the bond's width side) and normal (its direction) -
 *  see that shader for how those become an actual on-screen width. */
class BondRod : public Image
{
public:
	BondRod(const std::string &path);

	void fixVertices(const glm::vec3 &start, const glm::vec3 &dir);
};

#endif
