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

#ifndef __vagabond__ViewCorrelations__
#define __vagabond__ViewCorrelations__

#include <vagabond/gui/elements/Scene.h>
#include <vagabond/core/Item.h>
#include <vagabond/utils/Eigen/Dense>
#include <mutex>
#include <atomic>
#include <memory>
#include <list>

class Correlative;
class Clique;

class ViewCorrelations : public Scene
{
public:
	ViewCorrelations(Scene *prev, Clique *clique);
	~ViewCorrelations();

	virtual void setup();
	void makeList();
	void viewSubnetwork(Clique &clique);
	void occupancies();
	void viewAll();
private:
	// second half of viewAll() - runs once the (cancellable) assembly of
	// _result/_correlative below has completed on the main thread.
	void finishAssembly();

	Clique *_clique{};
	Correlative *_correlative{};

	// shared with whichever background assembly is currently in flight (if
	// any), so setBackJob() can flag it to stop early without needing to
	// touch anything Scene-specific from that background thread.
	std::shared_ptr<std::atomic<bool>> _cancelled;

	// true from the moment viewAll() kicks off a background assembly
	// until that assembly's last exit point (finishAssembly(), or one of
	// the cancelled-early returns) - guards against a second overlapping
	// run, e.g. from clicking the top clique's list entry while the
	// auto-triggered assembly from setup() is still in flight.
	bool _assembling = false;

	// makeList() borrows _clique's and its subdivisions' select jobs
	// (shared Item state, not view-local) for the duration this view is
	// open - saved here before being overwritten, and restored in the
	// destructor so nothing is left pointing at this Scene once it closes.
	std::map<Clique *, std::function<void(bool)>> _prevJobs;

	Eigen::MatrixXf _matrix{};
	std::mutex _mutex{};
	Eigen::MatrixXf _result{};

	// viewSubnetwork()'s grid of per-pair MatrixPlots each hold a live
	// reference into one of these - a std::list so pushing new entries
	// during that loop never invalidates references already handed out
	// to earlier MatrixPlots (unlike std::vector, which can reallocate).
	// Cleared alongside deleteTemps() at the top of viewSubnetwork(),
	// once the MatrixPlots referencing the previous run's entries are
	// themselves gone.
	std::list<Eigen::MatrixXf> _subnetworkMats;
};

#endif
