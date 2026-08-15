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

#ifndef __vagabond__OccupanciesView__
#define __vagabond__OccupanciesView__

#include <vagabond/gui/elements/IndexResponseView.h>
#include <vagabond/core/protonic/Energy.h>
#include <unordered_map>
#include <memory>
#include <atomic>

class Graph;
class Clique;
class Network;
class Slider;
class Image;
class Text;
struct ProbeTypePair;

class OccupanciesView : public IndexResponseView
{
public:
	OccupanciesView(Scene *prev, Clique *clique, Network &network);
	~OccupanciesView();

	virtual void setup();
	void occupancies();
	
	struct OccData
	{
		float calculated{};
		float observed{};
		float probs{};
		size_t samples{};
	};

protected:
	virtual void interactedWithNothing(bool left, bool hover = false);
private:
	void slider(std::string msg, const hnet::Energy::Source &src, float y);

	// updates _phValueText's label and _phWarning's visibility for the
	// given test pH - shared by the slider's own drag job and setup()
	// (so the warning icon already reflects reality if this Network's
	// testPH was left set by a previous visit to this screen).
	void updatePhDisplay(float pH);

	// sweeps effectivePH() from 0 to 14 in 0.25 steps (57 points),
	// leaving the energy-source toggles/amplifiers exactly as they
	// currently are, recomputing estimates() and its correlation at each
	// point on a background thread (behind a progress bar - a single
	// estimates() pass is fast enough for one manual pH nudge, but 57 in
	// a row noticeably isn't), then opens a GraphView line plot of
	// correlation vs pH. Restores the pre-scan pH afterwards - this is a
	// read-only exploratory sweep, not meant to leave the slider
	// somewhere the user didn't put it. Doesn't touch _lastEstimates/
	// _graph, so the scatter plot on screen is untouched throughout.
	void scanPH();

	typedef std::map<ProbeTypePair, OccData> EstimateMap;
	EstimateMap estimates();

	// (re)builds _graph from _lastEstimates, applying the current
	// _showWaters/_cliqueOnly filters - shared by occupancies() (which
	// recomputes _lastEstimates first) and the top-left tickboxes' own
	// toggle jobs (which just re-filter the same, already-computed
	// estimates, since neither filter changes what estimates() itself
	// would produce). A no-op if occupancies() has never actually been
	// run yet (_lastEstimates empty) - the tickboxes are pure display
	// filters on top of an existing plot, not their own trigger to run
	// the (potentially slow) full estimation pass.
	void rebuildGraph();

	Clique *_clique{};

	Graph *_graph{};
	Network &_network;

	EstimateMap _lastEstimates;

	// top-left display-filter tickboxes (see setup()/rebuildGraph()).
	bool _showWaters = true;
	bool _cliqueOnly = false;

	// live pH nudge (Network::setTestPH()) - deliberately doesn't touch
	// Model/CustomProtonSettings, so it never marks EditModel's
	// "outdated" icon and is never persisted (see setTestPH()'s own
	// comment). Existing energy-source probes already re-score live on
	// "Check occupancies" - this just makes the protonation term's own
	// contribution follow the slider rather than staying pinned to
	// whatever pH the network was actually built at.
	Text *_phValueText = nullptr;
	Image *_phWarning = nullptr;

	// so scanPH()'s own completion job can move the dot back to the
	// restored pH once the sweep finishes.
	Slider *_phSlider = nullptr;

	// cancels a scan still running in the background if another one is
	// started, or if this view is torn down mid-scan - same shared_ptr
	// idiom ProtonNetworkView::buildNetwork() uses for its own background
	// job.
	std::shared_ptr<std::atomic<bool>> _scanCancelled;
};

#endif
