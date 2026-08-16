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

#ifndef __vagabond__SubdivisionRunDetails__
#define __vagabond__SubdivisionRunDetails__

#include <vagabond/gui/elements/Modal.h>
#include <functional>

class Clique;
class SubdivisionRun;

/** what settings a Subdivide::subdivide()/one() run used and what it
 * produced (clique count, average node count) - opened by clicking a
 * run's name in HBondAnalysisControl's subdivision-runs list - plus a
 * delete button so a run can be discarded without disturbing any other
 * stored run. */
class SubdivisionRunDetails : public Modal
{
public:
	SubdivisionRunDetails(Scene *scene, Clique *clique, SubdivisionRun *run,
	                      const std::function<void()> &onChange);

	virtual void buttonPressed(std::string tag, Button *button = nullptr);
private:
	Clique *_clique;
	SubdivisionRun *_run;
	std::function<void()> _onChange;
};

#endif
