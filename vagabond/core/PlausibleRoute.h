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

#ifndef __vagabond__PlausibleRoute__
#define __vagabond__PlausibleRoute__

#include "Route.h"
#include "Progressor.h"

class PlausibleRoute : public Route, public Progressor
{
public:
	PlausibleRoute(Molecule *mol, Cluster<MetadataGroup> *cluster, int dims);

	void setDestination(Point dest);
	
	void calculateLinearProgression(int steps);
	virtual void setup();
protected:
	virtual void doCalculations();
	bool flipTorsions(bool main = false);
	bool nudgeWaypoints();
private:
	void addLinearInterpolatedPoint(float frac);
	float getTorsionAngle(int i);
	float momentumScore(int steps);
	void startTicker(std::string tag);

	Point _destination;
	
	struct WayPoint
	{
		float fraction = 0.5; /* fraction of route progression between 0 and 1 */
		float progress = 0.5; /* proportion of progress to goal at WayPoint */
		
		static WayPoint startPoint()
		{
			WayPoint wp;
			wp.fraction = 0;
			wp.progress = 0;
			return wp;
		}
		
		static WayPoint midPoint()
		{
			WayPoint wp;
			wp.fraction = 0.5;
			wp.progress = 0.5;
			return wp;
		}
		
		static WayPoint endPoint()
		{
			WayPoint wp;
			wp.fraction = 1;
			wp.progress = 1;
			return wp;
		}
	};
	
	typedef std::vector<WayPoint> WayPoints;
	
	/* int: referring to torsion angle via _destination[int] */
	std::map<int, WayPoints> _wayPoints;
	std::vector<bool> _flips;
	
	float _magnitudeThreshold = 90.f;
};

#endif
