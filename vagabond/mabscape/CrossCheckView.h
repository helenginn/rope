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

#ifndef __vagabond__CrossCheckView__
#define __vagabond__CrossCheckView__

#include <vagabond/gui/elements/Scene.h>
#include "PosToComp.h"

class Positions;
struct Competition;

/** summary statistics of a group of Results, averaged as the Playground
 *  does (see Positions::applyAverage()): how many went into the average, how
 *  far the results scatter around it, and how close it lies to the known
 *  structural position of any fiducials that were left free. */
class CrossCheckView : public Scene
{
public:
	struct Stats
	{
		size_t results{};
		size_t antibodies{}; // which were free to move in these results
		// Angstroms, deviation of every antibody position from its average,
		// per result: mean and standard deviation over the results
		float rmsd{};
		float rmsdSd{};
		// fiducials with structural information which were free in these
		// results, and the distance of each one's average position from its
		// structural position (under its best symmetry operator)
		struct Freed
		{
			std::string name;
			float distance{};
			// the same distance taken result by result: spread over the results
			float sd{};
			// how many of the data matrix elements which involve this antibody
			// have a value (the rest are missing)
			size_t elements{};
		};

		std::vector<Freed> freed;
		float freedMean{};
		float freedSd{};
		// what chance would give for that distance: two random mesh vertices,
		// allowing for symmetry
		float randomDistance{};
	};

	// key: the fixed antibodies of the results' bucket, "X, Y" as in
	// ResultsView::bucketKey()
	static Stats compute(const Positions &positions, Competition &comp,
	                     const std::string &key,
	                     const std::vector<PosToComp::Result> &results);

	CrossCheckView(Scene *prev, const std::string &title, const Stats &stats);

	virtual void setup();
private:
	std::string _title;
	Stats _stats;
};

#endif
