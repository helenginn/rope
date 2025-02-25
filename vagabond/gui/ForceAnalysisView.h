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

#ifndef __vagabond__ForceAnalysisView__
#define __vagabond__ForceAnalysisView__

#include <vagabond/gui/Display.h>
#include <vagabond/core/forces/ForceAnalysis.h>

class AtomContent;
class Rod;
class Torque;
class Magnitude;
class ShowAbstractForce;

class ForceAnalysisView : public Display
{
public:
	ForceAnalysisView(Scene *prev, AtomContent *atoms);

	virtual void setup();
	
	void handleTorque(Particle *p, Rod *rod, Torque *torque, 
	                  float magnitude, bool invert);
	void handleForce(Particle *p, Force *force, float magnitude);
private:
	void updateForces();

	ForceAnalysis _analysis;
	
	std::vector<ShowAbstractForce *> _shows;

};

#endif
