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

#include "ViewCorrelations.h"

#include <vagabond/utils/Eigen/Core>
#include <vagabond/utils/FloydWarshall.h>
#include <vagabond/utils/DoJob.h>
#include <fstream>
#include <vagabond/core/protonic/Clique.h>
#include <vagabond/core/protonic/Correlative.h>
#include <vagabond/core/protonic/CertainStates.h>
#include <vagabond/gui/elements/list/LineGroup.h>
#include <vagabond/gui/elements/AskYesNo.h>
#include <vagabond/gui/elements/AskForText.h>
#include <vagabond/gui/elements/BadChoice.h>
#include <vagabond/gui/elements/ImageButton.h>
#include <vagabond/gui/elements/TextButton.h>
#include <vagabond/gui/elements/Menu.h>
#include <vagabond/gui/elements/Text.h>
#include <vagabond/gui/elements/Image.h>
#include <vagabond/gui/MatrixPlot.h>
#include <vagabond/gui/ClusterPlot.h>
#include <vagabond/gui/HBondDiagram.h>
#include <vagabond/gui/CommunicationChoice.h>
#include <vagabond/gui/CommunicationAnalysis.h>
#include <vagabond/gui/elements/ScrollBox.h>
#include <vagabond/gui/elements/Window.h>
#include <vagabond/gui/VagWindow.h>
#include <vagabond/core/Progressor.h>
#include <vagabond/core/protonic/Probe.h>
#include <vagabond/core/Atom.h>
#include <set>
#include <map>
#include <cctype>
#include <cmath>
#include <algorithm>

using Eigen::seqN;

namespace
{
	// re-maps every distinct value in dist so that, sorted ascending, no
	// two consecutive distinct values are more than maxJump apart - only
	// display-shaping, done here rather than inside ClusterPlot itself
	// (which deliberately has no notion of what a distance matrix means,
	// per its own header comment) or CertainStates::distanceMatrix()
	// (whose raw label-difference counts other callers, e.g. correlate(),
	// still need untouched).
	//
	// Label-difference counts can span a huge range once a subnetwork has
	// many states (most pairs sharing almost nothing in common vs. a few
	// near-identical pairs), and ClusterPlot's own target scaling
	// (_displayScale, picked so the single widest pairwise distance maps
	// to about half a screen unit) means a handful of extreme outlier
	// pairs squash every other, closer pair down towards the same tiny
	// on-screen separation - exactly the "hard to distinguish internal
	// features" this compresses away. Shrinking any gap wider than
	// maxJump down to exactly maxJump (by subtracting the same running
	// excess from every value beyond it) leaves already-small gaps
	// (<= maxJump) completely untouched and keeps the whole mapping
	// strictly order-preserving - it just stops one or two very distant
	// pairs from dominating the scale the rest get squeezed into.
	Eigen::MatrixXf capDistanceJumps(const Eigen::MatrixXf &dist,
	                                 float maxJump)
	{
		std::set<float> uniqueVals;
		for (int i = 0; i < dist.rows(); i++)
		{
			for (int j = 0; j < dist.cols(); j++)
			{
				uniqueVals.insert(dist(i, j));
			}
		}

		std::map<float, float> remap;
		float prevOld = 0.f;
		float prevNew = 0.f;
		bool first = true;

		for (float v : uniqueVals)
		{
			if (first)
			{
				remap[v] = v;
				prevOld = v;
				prevNew = v;
				first = false;
				continue;
			}

			float gap = v - prevOld;
			float cappedGap = std::min(gap, maxJump);
			prevNew += cappedGap;
			remap[v] = prevNew;
			prevOld = v;
		}

		Eigen::MatrixXf result(dist.rows(), dist.cols());
		for (int i = 0; i < dist.rows(); i++)
		{
			for (int j = 0; j < dist.cols(); j++)
			{
				result(i, j) = remap[dist(i, j)];
			}
		}

		return result;
	}
}

ViewCorrelations::ViewCorrelations(Scene *prev, Clique *clique)
: Scene(prev), Mouse2D(prev), IndexResponseView(prev), _clique(clique)
{

}

void ViewCorrelations::setup()
{
	IndexResponseView::setup();
	setMakesSelections();

	addTitle("Sub-network correlations");

	makeList();

	setBackJob([this]()
	{
		if (_cancelled)
		{
			_cancelled->store(true);
		}
		VagWindow::window()->requestProgressBarRemoval();
	});

	viewAll();
}

void ViewCorrelations::sendSelection(float t, float l, float b, float r,
                                     bool inverse)
{
	IndexResponseView::sendSelection(t, l, b, r, inverse);
	captureStateSelection();
}

void ViewCorrelations::sendClickSelection(double x, double y, bool inverse)
{
	IndexResponseView::sendClickSelection(x, y, inverse);
	captureStateSelection();
}

void ViewCorrelations::mouseMoveEvent(double x, double y)
{
	if (_dragged)
	{
		Scene::mouseMoveEvent(x, y);
		return;
	}

	Mouse2D::mouseMoveEvent(x, y);
}

void ViewCorrelations::mouseReleaseEvent(double x, double y,
                                         SDL_MouseButtonEvent button)
{
	Mouse2D::mouseReleaseEvent(x, y, button);

	// see this method's own header comment - a stuck _right silently
	// turns every later mouse move anywhere in this Scene into an
	// ongoing Mouse2D::mouseMoveEvent() z-zoom (the same translation a
	// real right-drag applies), compounding tick after tick until
	// whatever is on screen - including a totally unrelated, later-viewed
	// subnetwork's own HBondDiagram - has shrunk to a speck in the middle
	// of the window. Calling this again is always safe even when the
	// base class's own call already ran (same button, same false) - it
	// only ever writes _left/_right to match the button that just
	// released, never anything conditional on prior state.
	interpretMouseButton(button, false);
}

void ViewCorrelations::captureStateSelection()
{
	// only the per-subnetwork state-clustering plot's own selection is
	// meaningful here (see _lastSelectedStates's own comment) -
	// _activeClusterPlot is reused for showSubnetworkClustering() too (a
	// different, unrelated selection of subnetworks rather than states),
	// which must not overwrite this.
	if (_subnetworkView == SubnetworkView::StateClustering && _activeClusterPlot)
	{
		// selectedIndices() are plot-local (0..pointCount()-1), not
		// necessarily true state indices once _includedStates has gated
		// the plot down to fewer than every state - _plottedStateIndices
		// (filled in by whichever showStateClustering() call last built
		// this same plot) translates back.
		_lastSelectedStates.clear();
		for (int plotIdx : _activeClusterPlot->selectedIndices())
		{
			if (plotIdx >= 0 && plotIdx < (int)_plottedStateIndices.size())
			{
				_lastSelectedStates.push_back(_plottedStateIndices[plotIdx]);
			}
		}
	}
}

void ViewCorrelations::interactedWithNothing(bool left, bool hover)
{
	if (hover)
	{
		return;
	}

	if (!left && _subnetworkView == SubnetworkView::StateClustering &&
	    _activeClusterPlot)
	{
		showStateContextMenu();
	}
}

void ViewCorrelations::showStateContextMenu()
{
	Menu *menu = new Menu(this);
	menu->addOption("Exclude unselected", [this]() { excludeUnselectedStates(); });
	menu->addOption("Invert selection", [this]() { invertStateSelection(); });
	menu->addOption("Reset selection", [this]() { resetStateSelection(); });

	float x; float y;
	getFractionalPos(x, y);
	menu->setup(x, y);
	setModal(menu);
}

void ViewCorrelations::invertStateSelection()
{
	if (!_activeClusterPlot)
	{
		return;
	}

	OpSet<int> selected(_activeClusterPlot->selectedIndices());
	int n = (int)_plottedStateIndices.size();
	for (int i = 0; i < n; i++)
	{
		_activeClusterPlot->selected(i, selected.count(i) > 0);
	}

	captureStateSelection();
}

void ViewCorrelations::excludeUnselectedStates()
{
	if (!_viewedSubnetwork)
	{
		return;
	}

	// makes sure _lastSelectedStates reflects whatever is on screen right
	// now, in case this fires without an intervening sendSelection()/
	// sendClickSelection() call (it shouldn't, but this is cheap enough
	// to not rely on that).
	captureStateSelection();

	if (_lastSelectedStates.empty())
	{
		// nothing selected to narrow down to - a no-op rather than
		// excluding every state.
		return;
	}

	_includedStates = _lastSelectedStates;
	_lastSelectedStates.clear();

	// same rebuild path viewSubnetwork() itself uses - _viewedSubnetwork
	// is already this same subnetwork, so its own "different subnetwork"
	// guard leaves _lastSelectedStates (just cleared above) alone.
	viewSubnetwork(*_viewedSubnetwork);
}

void ViewCorrelations::resetStateSelection()
{
	if (!_viewedSubnetwork)
	{
		return;
	}

	_includedStates.clear();
	_lastSelectedStates.clear();

	viewSubnetwork(*_viewedSubnetwork);
}

ViewCorrelations::~ViewCorrelations()
{
	for (auto &pair : _prevJobs)
	{
		pair.first->setSelectJob(pair.second);
	}

	// _overall/_written inside here are full N x N matrices (N = summed
	// dim_for_type() over every distinct ProbeTypePair across all
	// subdivisions) - never freed otherwise, so every visit to this view
	// on a clique with many subdivisions leaked several hundred MB to
	// low-GB permanently once the view closed.
	delete _correlative;
}

void ViewCorrelations::makeList()
{
	_prevJobs[_clique] = _clique->selectJob();
	_clique->setSelectJob({});

	for (Clique &clique : _clique->subdivisions())
	{
		auto view_subnetwork = [this, &clique](bool left)
		{
			if (left)
			{
				viewSubnetwork(clique);
			}
		};

		_prevJobs[&clique] = clique.selectJob();
		clique.setSelectJob(view_subnetwork);
	}

	_clique->setSelectJob([this](bool left) { if (left) viewAll(); });
	std::cout << ("Subgroups of " +
	                        std::to_string(_clique->probes().size())
	                        + " nodes") << std::endl;

	// collapsed by default - viewAll() now runs automatically on open (see
	// setup()), so the master clique's own list entry is mostly redundant
	// as a click target, and a clique with many subdivisions produces one
	// ItemLine/LineGroup per subdivision, which is slow and overwhelming
	// to render all expanded. Still just a toggle, not permanentCollapse()
	// - clicking it un-collapses like any other entry.
	_clique->collapse();

	LineGroup *lg = new LineGroup(_clique, this);
	// was -0.04 - pushed the top-level ("parent") row's own left edge
	// past the true screen edge, cutting its own name off; every child
	// row (indented further right - see ItemLine::addBranch()/addArrow())
	// stayed clear of it, which is why only the parent row was affected.
	lg->setLeft(0.0, 0.2);
	std::cout << "Clique count: " << _clique->itemCount() << std::endl;

	ScrollBox *sb = new ScrollBox();
	sb->setContent(lg);
	lg->setScrollBox(sb);
	sb->setBounds(glm::vec4(0.15, 0.0, 0.9, 0.28));
	addObject(sb);

	lg->refreshGroups();
	sb->addSliderIfNeeded();

	makeSearchButton(lg);
}

void ViewCorrelations::makeSearchButton(LineGroup *lg)
{
	// everything this search needs to remember between separate clicks
	// (the current query text, and the label/cross widgets while a search
	// is active) lives in this closure rather than as ViewCorrelations
	// members - nothing outside this wiring ever needs to reach it.
	struct SearchState
	{
		std::string text;
		Text *label = nullptr;
		ImageButton *cancel = nullptr;
	};
	auto state = std::make_shared<SearchState>();

	// declared before it's assigned so the body below - which needs to
	// hand a way of re-invoking itself (with "") to the cross button it
	// creates - can capture it by value.
	auto apply = std::make_shared<std::function<void(std::string)>>();

	*apply = [this, lg, state, apply](std::string text)
	{
		std::vector<ResidueRangeToken> tokens;

		if (text.size())
		{
			std::string error;
			if (!parseResidueRanges(text, tokens, error))
			{
				BadChoice *bc = new BadChoice(this, error);
				setModal(bc);
				return;
			}
		}

		state->text = text;
		_clique->setSearchText(text);

		tokens = resolveChains(tokens);
		filterSubdivisions(*_clique, tokens);
		lg->refreshGroups();

		if (state->text.empty())
		{
			if (state->label)
			{
				removeObject(state->label);
				delete state->label;
				state->label = nullptr;
			}

			if (state->cancel)
			{
				removeObject(state->cancel);
				delete state->cancel;
				state->cancel = nullptr;
			}
		}
		else
		{
			if (!state->label)
			{
				state->label = new Text(state->text);
				state->label->resize(0.6);
				state->label->setRight(0.90, 0.06);
				addObject(state->label);
			}
			else
			{
				state->label->setText(state->text);
			}

			if (!state->cancel)
			{
				state->cancel = new ImageButton("assets/images/cross.png",
				                                this);
				state->cancel->resize(0.06);
				state->cancel->setRight(0.93, 0.06);
				state->cancel->setReturnJob([apply]() { (*apply)(""); });
				addObject(state->cancel);
			}
		}

		viewChanged();
	};

	ImageButton *b = new ImageButton("assets/images/search.png", this);
	b->resize(0.1);
	b->setRight(0.97, 0.06);
	b->setReturnJob([this, state, apply]()
	{
		AskForText *aft = new AskForText(this, "Search for residue(s):",
		                                 "", this);
		aft->allowCapitals(true);
		aft->setHelpText("Hide sub-networks not containing a residue:\n\n"\
		                 "e.g. 115, A115, B145, 109-111\n"\
		                 "Comma-separated residues - a dash indicates a "\
		                 "range,\nand a chain letter applies until you "\
		                 "give a new one.\nNo chain letter defaults to "\
		                 "the first chain.\n\n"\
		                 "Leave blank and confirm to clear the search.");
		aft->setDefaultText(state->text);
		aft->setReturnJob([apply](std::string text) { (*apply)(text); });
		setModal(aft);
	});
	addObject(b);

	// re-displays the search text/cross-button indicator on re-entering
	// this list (see Clique::searchText()'s own comment) - also
	// harmlessly re-applies the same filter via filterSubdivisions(), but
	// that was already in effect (it lives on the Clique tree itself), so
	// this is really only about restoring the visual indicator.
	if (_clique->searchText().length())
	{
		(*apply)(_clique->searchText());
	}
}

std::vector<ResidueRangeToken> ViewCorrelations::resolveChains(
const std::vector<ResidueRangeToken> &tokens)
{
	// std::map keeps chain names in sorted (alphabetical) order, which is
	// relied on below for "first chain that actually has this residue".
	std::map<std::string, std::set<int>> residuesByChain;

	for (Probe *const &probe : _clique->probes())
	{
		if (probe->is_atom() && probe->atom())
		{
			Atom *atom = probe->atom();
			residuesByChain[atom->chain()].insert(atom->residueNumber());
		}
	}

	auto sameIgnoreCase = [](const std::string &a, const std::string &b)
	{
		if (a.size() != b.size())
		{
			return false;
		}

		for (size_t i = 0; i < a.size(); i++)
		{
			if (std::tolower((unsigned char)a[i]) !=
			    std::tolower((unsigned char)b[i]))
			{
				return false;
			}
		}

		return true;
	};

	// first chain (alphabetically) with an atom whose residue number
	// falls in [begin, end] - not just the first chain outright, which
	// may never reach that residue at all (e.g. a water chain). "" (never
	// equal to a real chain) if no chain has it.
	auto firstChainWithResidue = [&residuesByChain](int begin, int end)
	{
		for (const auto &entry : residuesByChain)
		{
			for (int res = begin; res <= end; res++)
			{
				if (entry.second.count(res))
				{
					return entry.first;
				}
			}
		}

		return std::string();
	};

	std::vector<ResidueRangeToken> resolved = tokens;

	for (ResidueRangeToken &token : resolved)
	{
		if (token.chain.empty())
		{
			token.chain = firstChainWithResidue(token.begin, token.end);
			continue;
		}

		if (residuesByChain.count(token.chain))
		{
			continue;
		}

		// user-proofing: e.g. typed "a115" but the structure only has
		// chain "A" - correct to the chain that actually exists rather
		// than silently matching nothing.
		for (const auto &entry : residuesByChain)
		{
			if (sameIgnoreCase(entry.first, token.chain))
			{
				token.chain = entry.first;
				break;
			}
		}
	}

	return resolved;
}

bool ViewCorrelations::cliqueMatchesTokens(Clique &clique,
                                           const std::vector<ResidueRangeToken>
                                           &tokens)
{
	auto probe_matches = [&tokens](Probe *const &probe)
	{
		if (!probe->is_atom() || !probe->atom())
		{
			return false;
		}

		const std::string &probeChain = probe->atom()->chain();
		int resNum = probe->atom()->residueNumber();

		for (const ResidueRangeToken &token : tokens)
		{
			if (probeChain == token.chain && resNum >= token.begin
			    && resNum <= token.end)
			{
				return true;
			}
		}

		return false;
	};

	for (Probe *const &probe : clique.probes())
	{
		if (probe_matches(probe))
		{
			return true;
		}
	}

	// clique.probes() is only this subnetwork's own original/core set -
	// ptps() (once a search has actually run) also includes whatever
	// peripheral nodes ExhaustiveSearch's own wider resolution pulled in,
	// which the search should be able to find too, not just the core.
	if (clique.states())
	{
		for (const ProbeTypePair &ptp : clique.states()->ptps())
		{
			if (ptp.first && probe_matches(ptp.first))
			{
				return true;
			}
		}
	}

	return false;
}

void ViewCorrelations::filterSubdivisions(Clique &clique,
                                          const std::vector<ResidueRangeToken>
                                          &tokens)
{
	for (Clique &sub : clique.subdivisions())
	{
		bool visible = tokens.empty() || cliqueMatchesTokens(sub, tokens);
		sub.setHidden(!visible);
		filterSubdivisions(sub, tokens);
	}
}

void ViewCorrelations::viewAll()
{
	if (_assembling)
	{
		return;
	}
	_assembling = true;

	// wipes whatever the user was looking at before (e.g. a subnetwork's
	// own view, or a previous "All" screen) and shows the toggle right
	// away, rather than only once assembly finishes - stage 1 below is
	// already safely cancellable (see makeTopLevelViewToggle()'s own
	// comment on why stage 2 is handled differently).
	clearScreen();
	_correlationMatrixButton = nullptr;
	_subnetworkClusteringButton = nullptr;
	_matrixComplete = false;
	_stage2Running = false;

	// this (and finishAssembly(), further down) is what displays the
	// correlation matrix grid - but this is also the parent clique's own
	// list-entry click handler (see makeList()), reachable again later
	// even after the user had switched to Subnetwork clustering
	// (_topLevelView left stale at that value otherwise). Without this,
	// makeTopLevelViewToggle()'s active/inert computation (keyed off
	// _topLevelView) kept marking "Subnetwork clustering" as the current
	// (inert, unclickable) view even once this finished and the screen
	// was actually showing the correlation matrix - i.e. the toggle
	// looked stuck on "Subnetwork clustering" and refused to switch back
	// to it, despite that view no longer being shown at all.
	_topLevelView = TopLevelView::CorrelationMatrix;
	makeTopLevelViewToggle();

	{
		OpSet<Probe *> covered;
		for (const Clique &sub : _clique->subdivisions())
		{
			covered += sub.probes();
		}

		std::cout << "ViewCorrelations::viewAll(): clique has "
		          << _clique->probes().size() << " probes ("
		          << _clique->nonWaterProbes().size() << " non-water - "
		          << "the pool CommunicationChoice draws group members "
		          << "from), " << _clique->subdivisions().size()
		          << " subdivisions collectively covering "
		          << covered.size() << " distinct probes" << std::endl;
	}

	// cancellable via the back button (see setup()) - a mis-click into
	// this view shouldn't force the user to sit through assembling a
	// large clique's correlations just to leave again.
	auto cancelled = std::make_shared<std::atomic<bool>>(false);
	_cancelled = cancelled;

	struct AssemblyProgress : public Progressor {};
	AssemblyProgress *progress = new AssemblyProgress();

	auto cancelJob = [cancelled]()
	{
		cancelled->store(true);
	};

	Clique *clique = _clique;
	int ticks = (int)clique->subdivisions().size();
	VagWindow::window()->requestProgressBar(ticks, "Assembling correlations",
	                                        progress, cancelJob);

	auto assemble = [this, clique, cancelled, progress]()
	{
		float all_ave = 0;
		OpSet<ProbeTypePair> all =
		Correlative::probeTypePairs(clique->subdivisions(), all_ave);

		Correlative *correl = new Correlative(all, all_ave, false);

		for (Clique &sub : clique->subdivisions())
		{
			if (cancelled->load())
			{
				break;
			}

			if (sub.states())
			{
				correl->addStates(*sub.states(), (float)sub.sampleWeight());
			}

			progress->clickTicker();
		}

		progress->finishTicker();

		if (cancelled->load())
		{
			delete progress;
			delete correl;
			_assembling = false;
			return;
		}

		Eigen::MatrixXf result = correl->acquireMatrix();

		// hands off to the main thread rather than touching this Scene's
		// state directly from the background thread - VagWindow (unlike
		// this Scene) is guaranteed to still be alive whenever this runs.
		VagWindow::window()->addMainThreadJob(
		[this, correl, result, cancelled, progress]()
		{
			delete progress;

			// checked again: cancellation may have landed after the loop
			// above finished but before this main-thread job got to run.
			if (cancelled->load())
			{
				delete correl;
				_assembling = false;
				return;
			}

			// viewAll() can run again on the same open Scene (e.g. the
			// list entry re-clicked) - free whatever the previous run
			// left behind rather than leaking its _overall/_written.
			delete _correlative;
			_correlative = correl;
			_result = result;
			_assembling = false;
			finishAssembly();
		});
	};

	new DoJob(assemble);
}

MatrixPlot *ViewCorrelations::buildCorrelationMatrixUI(bool addButton)
{
	MatrixPlot *mp = new MatrixPlot(_matrix, _mutex);

	auto lookup = _correlative->matrixLookup();
	auto display_lookup = [this, lookup](float x, float y)
	{
		std::string info = lookup(x, y);
		setInformation(info);
	};

	mp->setHoverJob(display_lookup);
	glm::mat3x3 rot = glm::mat3x3(1.f);
	rot[1] *= -1;
	mp->rotateRoundCentre(rot);
	Renderable::Alignment align = Renderable::Alignment
	(Renderable::Alignment::Left | Renderable::Alignment::Top);
	mp->setArbitrary(0.4, 0.25, align);
	addTempObject(mp);

	if (addButton)
	{
		addCommunicationAnalysisButton();
	}

	return mp;
}

void ViewCorrelations::addCommunicationAnalysisButton()
{
	auto choose_groups = [this]()
	{
		CommunicationChoice *cc = new CommunicationChoice(this, _clique);
		cc->show();
	};

	auto comm_analysis = [this, choose_groups]()
	{
		int num = _clique->allCommsNames().size();
		if (num <= 1)
		{
			AskYesNo *ayn =
			new AskYesNo(this, "Need to define at least two communication "\
			             "groups before analysis. Choose them now?", "", this);

			ayn->addJob("yes", choose_groups);
			setModal(ayn);
		}
		else
		{
			CommunicationAnalysis *ca =
			new CommunicationAnalysis(this, _clique, _result,
			                          _correlative->insertions());
			ca->show();
		}
	};

	TextButton *comm = new TextButton("Communication analysis", this);
	comm->setCentre(0.55, 0.9);
	comm->setReturnJob(comm_analysis);
	addTempObject(comm);
}

void ViewCorrelations::finishAssembly()
{
	// bridges the gap between the "Assembling correlations" bar finishing
	// and FloydWarshall's own "Deriving intermediate correlations" bar
	// appearing (requested from its own background thread, so not
	// instant) - without this, that gap has no visual feedback at all.
	setInformation("Matrix assembled - preparing to derive correlations");

	_matrix = _result;
	// addButton=false - the matrix FloydWarshall is about to gap-fill
	// isn't complete/analysable yet, so "Communication analysis" doesn't
	// belong here until fill_gaps's own completion below actually adds
	// it - previously this appeared the moment gap-filling *began*, not
	// once it finished.
	MatrixPlot *mp = buildCorrelationMatrixUI(false);

	// not cancellable - the comparison matrix isn't usable until this
	// finishes, so there is nothing sensible to fall back to if aborted.
	// makeTopLevelViewToggle() and viewSubnetwork() both refuse to
	// navigate away for exactly this window (see _stage2Running's own
	// comment) so nothing deletes mp while fw's background thread still
	// holds a raw pointer to it, calling update() on it.
	_stage2Running = true;

	struct GapFillProgress : public Progressor {};

	auto fill_gaps = [this, mp]()
	{
		auto combine = [](float x, float y)
		{
			return x * y;
		};

		setInformation("Deriving intermediate correlations");

		FloydWarshall fw(_result, combine, true);
		fw.addDisplayMatrix(_matrix, _mutex, [mp]() { mp->update(); });

		GapFillProgress *progress = new GapFillProgress();
		fw.addTickJob([progress]() { progress->clickTicker(); });

		VagWindow::window()->requestProgressBar((int)_result.rows(),
		                                        "Deriving intermediate "\
		                                        "correlations", progress);

		fw.run();
		progress->finishTicker();
		delete progress;

		setInformation("Finished deriving intermediates");

		addMainThreadJob([this]()
		{
			// lets showTopLevelView() reuse this run's _matrix/_correlative
			// instantly instead of re-running the whole assembly if the
			// user switches to subnetwork clustering and back - also
			// re-enables the "Subnetwork clustering" button, disabled
			// until now (see makeTopLevelViewToggle()).
			_matrixComplete = true;
			_stage2Running = false;
			makeTopLevelViewToggle();
			addCommunicationAnalysisButton();

			viewChanged();
		});
	};

	new DoJob(fill_gaps);
}

void ViewCorrelations::viewSubnetwork(Clique &clique)
{
	// unlike the top-level toggle buttons (see makeTopLevelViewToggle()),
	// the subnetwork list itself (makeList()) is a persistent object,
	// never gated on _assembling/_matrixComplete - reachable at any time,
	// including mid-assembly. Without cancelling here too, an in-flight
	// stage 1 run (viewAll()'s "assemble" DoJob) would keep computing in
	// the background and still land later via finishAssembly(), adding
	// its own MatrixPlot/button on top of whatever this subnetwork's own
	// view is showing by then - same mechanism the toggle buttons and
	// back button already use; a no-op if nothing is currently running.
	if (_cancelled)
	{
		_cancelled->store(true);
	}
	VagWindow::window()->requestProgressBarRemoval();

	// stage 2 (FloydWarshall gap-filling, "fill_gaps" in finishAssembly())
	// has no cancellation support at all, and its background thread holds
	// a raw pointer to the MatrixPlot finishAssembly() already added,
	// calling update() on it for as long as it runs - deleteTemps()-ing
	// that same MatrixPlot below would race that still-live thread. The
	// toggle buttons already refuse to switch away for exactly this
	// window (see _stage2Running's own comment); do the same here rather
	// than let this list bypass it.
	if (_stage2Running)
	{
		return;
	}

	// a state index only means anything relative to this specific
	// subnetwork's own CertainStates - carrying it over into a different
	// subnetwork (as opposed to just switching tabs on the same one,
	// which also comes through here - see makeSubnetworkViewToggle())
	// would apply a stale, meaningless selection to whatever unrelated
	// state that index happens to land on there. _includedStates (the
	// "Exclude unselected" gate) is exactly the same kind of subnetwork-
	// relative state index and needs the same guard - without this, a
	// gate set up in one subnetwork would silently narrow down (or,
	// depending on index magnitudes, just do nothing meaningful to) an
	// unrelated later subnetwork's own states.
	if (_viewedSubnetwork != &clique)
	{
		_lastSelectedStates.clear();
		_includedStates.clear();
	}

	_viewedSubnetwork = &clique;

	setInformation(clique.name());
	clearScreen();
	_subnetworkMats.clear();

	// defensive: if a background FloydWarshall pass (see finishAssembly())
	// is still running when the user navigates here, its completion will
	// still fire makeTopLevelViewToggle() later regardless of what's on
	// screen by then - without this, it would try to double-delete the
	// buttons deleteTemps() just queued above.
	_correlationMatrixButton = nullptr;
	_subnetworkClusteringButton = nullptr;

	makeSubnetworkViewToggle();
	showSubnetworkView(clique);
}

void ViewCorrelations::makeSubnetworkViewToggle()
{
	auto make_button = [this](const std::string &label,
	                          SubnetworkView mode, float x)
	{
		TextButton *tb = new TextButton(label, this);
		tb->resize(0.38);
		tb->setCentre(x, 0.205);

		bool active = (_subnetworkView == mode);
		tb->setInert(active, true);

		tb->setReturnJob([this, mode]()
		{
			if (_subnetworkView == mode || !_viewedSubnetwork)
			{
				return;
			}

			_subnetworkView = mode;
			viewSubnetwork(*_viewedSubnetwork);
		});

		addTempObject(tb);
	};

	make_button("Hydrogen-bonding diagram",
	           SubnetworkView::HydrogenBondDiagram, 0.425);
	make_button("State clustering", SubnetworkView::StateClustering, 0.55);
	make_button("Correlation matrix", SubnetworkView::CorrelationMatrix,
	           0.69);
}

void ViewCorrelations::showSubnetworkView(Clique &clique)
{
	switch (_subnetworkView)
	{
		case SubnetworkView::CorrelationMatrix:
		showCorrelationMatrix(clique);
		break;

		case SubnetworkView::StateClustering:
		showStateClustering(clique);
		break;

		case SubnetworkView::HydrogenBondDiagram:
		showHydrogenBondDiagram(clique);
		break;
	}
}

void ViewCorrelations::showStateClustering(Clique &clique)
{
	if (!clique.states())
	{
		return;
	}

	int fullCount = (int)clique.states()->state_count();

	// _includedStates (see its own comment) gates which states actually
	// get plotted - empty means every state is still in play, same as
	// before "Exclude unselected" existed.
	if (_includedStates.empty())
	{
		_plottedStateIndices.resize(fullCount);
		for (int i = 0; i < fullCount; i++)
		{
			_plottedStateIndices[i] = i;
		}
	}
	else
	{
		_plottedStateIndices = _includedStates;
	}

	int n = (int)_plottedStateIndices.size();

	Eigen::MatrixXf fullDist = clique.states()->distanceMatrix().cast<float>();
	Eigen::MatrixXf dist(n, n);
	for (int i = 0; i < n; i++)
	{
		for (int j = 0; j < n; j++)
		{
			dist(i, j) = fullDist(_plottedStateIndices[i], _plottedStateIndices[j]);
		}
	}
	dist = capDistanceJumps(dist, 5.f);

	float ave = 0;
	std::vector<float> fullProbs = clique.states()->probsForLocalAve(ave);
	std::vector<float> probs(n);
	for (int i = 0; i < n; i++)
	{
		probs[i] = fullProbs[_plottedStateIndices[i]];
	}

	// default (unset) colour - box.fsh ADDS vertex colour to the texture
	// sample (result = texture(...) + vColor), and every vertex's colour
	// defaults to (0,0,0,0) (Vertex::color is zero-initialised) until
	// something calls setColour() - so (0,0,0) here is a true no-op,
	// leaving dot.png's own raw colour exactly as it renders everywhere
	// else. (1,1,1) was wrong: adding white brightens every channel
	// towards saturation regardless of the texture's actual colour.
	std::vector<glm::vec3> colours(n, glm::vec3(0.f));

	// only ever overwritten for a subnetwork whose own CertainStates
	// actually tracks the watched signal (_clique->watchedSignal(), set
	// via CommunicationChoice - _clique is the top-level clique this
	// whole screen was opened on, not this subnetwork), and even then
	// only for states that assign it a value at all - states().value()
	// returning -1 (this state's own local search never reached that
	// probe) leaves that state's colour at the default above, same
	// skip-logic CertainStates::distance() already uses.
	std::string watched = _clique->watchedSignal();
	bool foundWatched = false;
	Probe *watchedProbe = nullptr;
	hnet::Types watchedType = hnet::Types::Unassigned;

	// index 0 = Unassigned, 1..3 = label 0..2 (Absent/Present-or-Acceptor/
	// Donor) - probability mass (probs[i], the same per-state weight
	// ClusterPlot sizes dots by) summed per legend entry, so
	// makeStateClusteringLegend() can show what fraction of the total
	// each option accounts for.
	std::vector<float> labelSums(4, 0.f);
	float totalProbSum = 0.f;
	for (float p : probs)
	{
		totalProbSum += p;
	}

	if (watched.length())
	{
		const CertainStates &states = *clique.states();

		for (const ProbeTypePair &ptp : states.ptps())
		{
			if (!ptp.first || ptp.first->desc() != watched)
			{
				continue;
			}

			int row = states(ptp);
			if (row < 0)
			{
				break;
			}

			foundWatched = true;
			watchedProbe = ptp.first;
			watchedType = ptp.second;

			const glm::vec3 purple(0.4f, 0.f, 0.4f);
			const glm::vec3 orange(0.8f, 0.4f, 0.f);
			const glm::vec3 blue(0.2f, 0.2f, 0.8f);

			for (int i = 0; i < n; i++)
			{
				int raw = states.value(_plottedStateIndices[i], row);
				if (raw < 0)
				{
					labelSums[0] += probs[i];
					continue;
				}

				// raw is a bit-flag (Absent = 1<<0, Weak/Present = 1<<1,
				// Strong/Donor = 1<<2 - see hnet::Existence::Values/
				// hnet::Bond::Values) rather than already a plain 0/1/2
				// label - same decode CertainStates::correlate() uses
				// (its get_index lambda).
				int label = -1;
				for (int bit = 0; bit <= 2; bit++)
				{
					if (raw & (1 << bit))
					{
						label = bit;
						break;
					}
				}

				if (label >= 0)
				{
					labelSums[label + 1] += probs[i];
				}

				if (ptp.second == hnet::Types::ExistenceType)
				{
					if (label == 0) colours[i] = purple; // Absent
					else if (label == 1) colours[i] = orange; // Present
				}
				else if (ptp.second == hnet::Types::BondType)
				{
					if (label == 0) colours[i] = purple; // Absent
					else if (label == 1) colours[i] = orange; // Acceptor
					else if (label == 2) colours[i] = blue; // Donor
				}
			}

			break;
		}
	}

	ClusterPlot *cp = new ClusterPlot(dist, probs, getModel(), {}, colours);
	placeClusterPlot(cp);

	// a fresh ClusterPlot starts with nothing selected - reapply whatever
	// was selected last time this tab was visited (see
	// _lastSelectedStates's own comment), so switching to the H-bond
	// diagram tab and back doesn't silently drop the selection the user
	// made here. _lastSelectedStates holds true state indices, but a
	// gated plot's own node indices are plot-local (see
	// _plottedStateIndices's own comment) - translate via a reverse
	// lookup; a highlighted state that "Exclude unselected" has since
	// gated out simply has nothing to highlight any more, which is
	// correct.
	for (int idx : _lastSelectedStates)
	{
		auto found = std::find(_plottedStateIndices.begin(),
		                       _plottedStateIndices.end(), idx);
		if (found != _plottedStateIndices.end())
		{
			int plotIdx = (int)(found - _plottedStateIndices.begin());
			cp->selected(plotIdx, false);
		}
	}

	if (foundWatched)
	{
		makeStateClusteringLegend(watchedProbe, watchedType, labelSums,
		                         totalProbSum);
	}
}

void ViewCorrelations::makeStateClusteringLegend(Probe *probe,
                                                 hnet::Types type,
                                                 const std::vector<float> &labelSums,
                                                 float totalProbSum)
{
	struct LegendEntry
	{
		glm::vec3 colour;
		std::string meaning;
	};

	// (0,0,0) first, matching showStateClustering()'s own default colour
	// (a true no-op against box.fsh's additive blend, i.e. dot.png's own
	// raw - dark - colour) - always shown, regardless of type, since any
	// state can fail to assign the watched signal at all.
	std::vector<LegendEntry> entries;
	entries.push_back({glm::vec3(0.f), "Unassigned"});

	if (type == hnet::Types::ExistenceType)
	{
		entries.push_back({glm::vec3(0.4f, 0.f, 0.4f), "Absent"});
		entries.push_back({glm::vec3(0.8f, 0.4f, 0.f), "Present"});
	}
	else if (type == hnet::Types::BondType)
	{
		entries.push_back({glm::vec3(0.4f, 0.f, 0.4f), "Absent"});
		entries.push_back({glm::vec3(0.8f, 0.4f, 0.f), "Acceptor"});
		entries.push_back({glm::vec3(0.2f, 0.2f, 0.8f), "Donor"});
	}

	// "A-Asn65 altconf A" for an atom (existence) signal - short_desc
	// format (Atom::shortResidueName()) plus which conformer this
	// specific Probe's existence is being watched for, since several
	// conformers of the same residue can each carry their own
	// independent uncertain existence. Bond signals have no single atom
	// to name this way - desc() is the closest stable identifier.
	std::string name;
	if (probe->is_atom() && probe->atom())
	{
		name = probe->atom()->shortResidueName() + " altconf " +
		std::string(1, probe->_conf);
	}
	else
	{
		name = probe->desc();
	}

	float y = 0.32f;
	Text *header = new Text(name);
	header->resize(0.5);
	header->setLeft(0.86, y);
	addTempObject(header);
	y += 0.045f;

	for (size_t i = 0; i < entries.size(); i++)
	{
		const LegendEntry &entry = entries[i];

		Image *swatch = new Image("assets/images/dot.png");
		swatch->resize(0.018);
		swatch->setLeft(0.86, y);
		swatch->setColour(entry.colour.x, entry.colour.y, entry.colour.z);
		addTempObject(swatch);

		// entries[] is built in exactly the same order as labelSums
		// (index 0 = Unassigned, 1..3 = label 0..2), so its own loop
		// index doubles as the labelSums lookup.
		std::string meaning = entry.meaning;
		if (totalProbSum > 0.f)
		{
			int pct = (int)std::round(100.f * labelSums[i] / totalProbSum);
			meaning += " (" + std::to_string(pct) + "%)";
		}

		Text *label = new Text(meaning);
		label->resize(0.45);
		label->setLeft(0.89, y);
		addTempObject(label);

		y += 0.04f;
	}
}

void ViewCorrelations::showHydrogenBondDiagram(Clique &clique)
{
	if (!clique.states())
	{
		return;
	}

	const CertainStates &states = *clique.states();

	float ave = 0;
	std::vector<float> baseline = states.probsForLocalAve(ave);
	std::vector<float> probs = baseline;

	// restrict the average to whichever states are currently selected on
	// the state-clustering tab (see _lastSelectedStates's own comment) -
	// zeroing out every other state's weight rather than building a
	// smaller probs vector, since proportions() indexes by raw state
	// index. Falls back to _includedStates (the state-clustering context
	// menu's "Exclude unselected" gate - see its own comment) if nothing
	// is currently highlighted, or every state (the plot's own default
	// weighting) if nothing has been excluded either. _lastSelectedStates,
	// whenever non-empty, is already a subset of _includedStates by
	// construction (only a currently-plotted, i.e. not-yet-excluded,
	// state can ever be highlighted), so no separate intersection is
	// needed.
	const std::vector<int> &restriction = !_lastSelectedStates.empty() ?
	_lastSelectedStates : _includedStates;
	if (!restriction.empty())
	{
		std::vector<float> restricted(probs.size(), 0.f);
		for (int idx : restriction)
		{
			if (idx >= 0 && idx < (int)restricted.size())
			{
				restricted[idx] = probs[idx];
			}
		}
		probs = restricted;
	}

	// same spot/positioning idiom as placeClusterPlot() (NOT setCentre()/
	// setArbitrary() - see that method's own comment on why) - HBondDiagram
	// isn't a ClusterPlot, so this doesn't go through that shared helper,
	// but uses the exact same target so all three subnetwork tabs land in
	// the same place.
	glm::vec3 target(2.f * 0.5f - 1.f + 0.25f, -(2.f * 0.6f - 1.f), 0.f);

	// one shared label, updated on hover rather than each atom showing
	// its own floating tag (see HoverAtomLabel's own comment in
	// HBondDiagram.cpp for why that doesn't work for a raw-setPosition()
	// atom) - horizontally centred on target.x, which stays the correct
	// midpoint of either a single diagram or a side-by-side pair (offset,
	// below, is symmetric around it). Constructed blank ("") deliberately
	// - Text's own constructor only builds real vertex geometry at all
	// when given non-empty text (see Text::Text()), so resize()/
	// setPosition() here would have nothing to act on yet regardless.
	// The real fix has to live in the callback below: Text::setText()
	// unconditionally rebuilds the quad from scratch at its natural size
	// and repositions it via realign() (the fractional setCentre()/
	// setLeft() layout system) - this label was never set up through
	// that system (raw setPosition(), same as every other position in
	// this method), so realign() falls back to its unset default (screen
	// origin) - hence both resize() and setPosition() have to be
	// reapplied after every single setText() call, not just once here.
	Text *hoverInfo = new Text("");
	addTempObject(hoverInfo);
	auto onHoverAtom = [hoverInfo, target](const std::string &desc)
	{
		hoverInfo->setText(desc);
		hoverInfo->resize(0.6);
		hoverInfo->setPosition(glm::vec3(target.x, -0.85f, 0.f));
		hoverInfo->forceRender(true, true);
	};

	// ptps() already only ever holds uncertain atoms plus non-covalent
	// bonds (ExhaustiveSearch skips is_covalent() probes entirely before
	// they ever reach CertainStates) - so this is naturally a hydrogen-
	// bonding diagram, with no separate filtering needed here. Always the
	// same (default) targetRadius regardless of side-by-side vs single -
	// see the offset calculation below for how the side-by-side case
	// instead makes room for two full-size diagrams.
	HBondDiagram *diagram = new HBondDiagram(states.ptps(), getModel(),
	                                         &states, probs, nullptr, {},
	                                         0.5f, onHoverAtom);

	// a second, side-by-side diagram showing the complement of the
	// selection (every other state) only makes sense once there is an
	// actual selection to contrast against - with nothing selected, probs
	// above is already every state, and its own "complement" would be
	// empty.
	if (_lastSelectedStates.empty())
	{
		diagram->setPosition(target);
		addTempObject(diagram);
		diagram->start();
		return;
	}

	// settle diagram's own physics synchronously (see PositionShifter::
	// settle() itself) before ever deciding screen placement from it - an
	// earlier attempt derived the offset from displayScale() instead (a
	// proxy for how much post-seed repulsion growth to expect), which
	// gave every subnetwork a different, unpredictable gap rather than a
	// consistent one; measuring the actual settled radius directly is
	// what that was trying to approximate in the first place. 200
	// iterations is a balance between an actually-relaxed layout and not
	// stalling the GUI thread noticeably - retune if diagrams still look
	// visibly unsettled the moment this tab opens.
	diagram->settle(200);

	// offset = measuredRadius() + a fixed margin guarantees the two
	// diagrams' own content (each up to measuredRadius() from its own
	// centre) cannot overlap (their centres end up 2*offset apart), while
	// still tracking each specific subnetwork's own actual size rather
	// than an arbitrary constant. Does not on its own guarantee staying
	// within the screen edges for a very large/sprawling subnetwork -
	// only that the pair itself is well laid out relative to each other.
	const float MARGIN = 0.05f;
	float offsetX = diagram->measuredRadius() + MARGIN;
	glm::vec3 offset(offsetX, 0.f, 0.f);
	diagram->setPosition(target - offset);

	// complement of _lastSelectedStates - the same baseline probs values
	// (this clique's own probsForLocalAve() weights, not just a flat 1/0
	// mask) restricting probs (above) started from, just zeroing out the
	// opposite set of states. baseline is every state CertainStates knows
	// about, regardless of _includedStates - without also zeroing out
	// anything "Exclude unselected" has since gated out, a gated-out
	// state's own weight would still land in this diagram's own
	// denominator (proportions()'s sum) even though it no longer even
	// appears in the state-clustering plot, silently pulling every
	// fraction here down towards whatever that invisible state's own
	// values happened to be.
	std::vector<float> complementProbs = baseline;
	OpSet<int> selected(_lastSelectedStates);
	OpSet<int> included(_includedStates);
	bool gated = !_includedStates.empty();
	for (size_t i = 0; i < complementProbs.size(); i++)
	{
		bool excluded = gated && !included.count((int)i);
		if (selected.count((int)i) || excluded)
		{
			complementProbs[i] = 0.f;
		}
	}

	// negative shares diagram's own PositionShifter (positionSource - see
	// HBondDiagram's own constructor comment) rather than running its own
	// physics, so it must be fully constructed - every addTidy()/
	// addPosition() call it makes on that shared shifter included -
	// before diagram->start() below ever spawns the physics thread that
	// reads from the same, unlocked _tidyJobs list (PositionShifter::
	// addTidy() has no locking at all - a start() before this would race
	// the two). positionOffset (2x offset - the difference between
	// negative's own target below and diagram's own target above) is
	// what actually keeps negative's own synced nodes offset from
	// diagram's rather than landing directly on top of them - see
	// HBondDiagram's own constructor/syncPositionsFrom() comments.
	HBondDiagram *negative = new HBondDiagram(states.ptps(), getModel(),
	                                          &states, complementProbs,
	                                          diagram, offset * 2.f,
	                                          0.5f, onHoverAtom);
	negative->setPosition(target + offset);

	addTempObject(diagram);
	addTempObject(negative);

	// one caption per diagram, just below it - smaller than hoverInfo
	// above (a static label, not the thing the user is actively reading
	// off), same x as each diagram's own setPosition() target above so
	// it stays centred underneath regardless of how offset ends up
	// scaling.
	auto add_caption = [this](const std::string &text, float x)
	{
		Text *caption = new Text(text);
		caption->resize(0.4);
		caption->setPosition(glm::vec3(x, -0.72f, 0.f));
		addTempObject(caption);
	};
	add_caption("selected state average", (target - offset).x);
	add_caption("unselected state average", (target + offset).x);

	// deliberately no negative->start() - see HBondDiagram's own
	// constructor comment: it is driven entirely by diagram's own
	// physics thread instead of running its own, so the two stay laid
	// out identically and only differ in the data (alpha/icon) each one
	// shows.
	diagram->start();
}

void ViewCorrelations::placeClusterPlot(ClusterPlot *cp)
{
	// NOT setCentre()/setArbitrary(): its recursive addAlign()->realign()
	// cascade calls setPosition() directly on every child too, using each
	// child's own (uninitialized, default 0/0) _x/_y fraction fields -
	// harmless for children that manage layout via those fields (e.g.
	// ItemLine), but ClusterPlot's dots are positioned purely by
	// PositionShifter via raw setPosition() calls of their own, so every
	// single one was getting forced to the exact same computed screen
	// position, destroying the physics-driven layout entirely.
	// setPosition() alone computes one shared delta from the group's
	// current centroid and applies it via addToVertices() - a genuine
	// rigid group translation that preserves relative spread.
	glm::vec3 target(2.f * 0.5f - 1.f + 0.15f, -(2.f * 0.6f - 1.f), 0.f);
	cp->setPosition(target);

	// drops the previous plot's own registration (if any) first - by the
	// time a new one is being placed, the old ClusterPlot has always
	// already been queued for deletion by an earlier deleteTemps() call,
	// so leaving its registration in _responders would be a dangling
	// IndexResponder* the next selection interaction could walk into.
	clearResponders();
	addIndexResponder(cp);
	_activeClusterPlot = cp;

	addTempObject(cp);
	cp->start();
}

void ViewCorrelations::clearScreen()
{
	deleteTemps();

	// see this method's own header comment - deleteTemps() alone leaves
	// both of these dangling once whatever ClusterPlot they refer to is
	// actually destroyed.
	clearResponders();
	_activeClusterPlot = nullptr;
}

void ViewCorrelations::makeTopLevelViewToggle()
{
	// stage 1 (the correlation assembly itself, DoJob "assemble" in
	// viewAll() - typically the slower of the two) is already safely
	// cancellable via the same _cancelled flag the back button uses, so
	// switching away from it is fine. Stage 2 (FloydWarshall gap-filling
	// in finishAssembly()'s "fill_gaps") has no cancellation support at
	// all, and its background thread holds a raw MatrixPlot* it keeps
	// calling update() on for as long as it runs - switching away and
	// deleteTemps()-ing that same MatrixPlot while stage 2 is still
	// running would race exactly the class of bug fixed for
	// StateClusterPlot/ThickLine earlier (a background thread touching a
	// Renderable the main thread is concurrently freeing). So: disabled
	// only in the one specific window where stage 1 has hand off to
	// finishAssembly() (_assembling false) but stage 2 hasn't fully
	// finished yet (_matrixComplete false).
	// see this method's own comment (header) - cleans up whichever pair
	// it previously built itself, if a caller hasn't already wiped them
	// via deleteTemps() (and nulled these two members) since. Without
	// this, calling this a second time (e.g. from fill_gaps's completion,
	// purely to refresh "Subnetwork clustering"'s enabled state, with no
	// deleteTemps() in between since that would also wipe the matrix
	// plot/comm button) duplicated both buttons instead of replacing them.
	if (_correlationMatrixButton)
	{
		removeObject(_correlationMatrixButton);
		delete _correlationMatrixButton;
		_correlationMatrixButton = nullptr;
	}

	if (_subnetworkClusteringButton)
	{
		removeObject(_subnetworkClusteringButton);
		delete _subnetworkClusteringButton;
		_subnetworkClusteringButton = nullptr;
	}

	// stage 1 (the correlation assembly itself, DoJob "assemble" in
	// viewAll() - typically the slower of the two) is already safely
	// cancellable via the same _cancelled flag the back button uses, so
	// switching away from it is fine. Stage 2 (FloydWarshall gap-filling
	// in finishAssembly()'s "fill_gaps") has no cancellation support at
	// all, and its background thread holds a raw MatrixPlot* it keeps
	// calling update() on for as long as it runs - switching away and
	// deleteTemps()-ing that same MatrixPlot while stage 2 is still
	// running would race exactly the class of bug fixed for
	// StateClusterPlot/ThickLine earlier (a background thread touching a
	// Renderable the main thread is concurrently freeing). So: disabled
	// only while _stage2Running is true (see its own comment on why that
	// is not simply derived from _assembling/_matrixComplete).
	bool stage2Running = _stage2Running;

	auto make_button = [this, stage2Running](const std::string &label,
	                          TopLevelView mode, float x) -> TextButton *
	{
		TextButton *tb = new TextButton(label, this);
		tb->resize(0.45);
		tb->setCentre(x, 0.205);

		bool active = (_topLevelView == mode);
		bool blocked = (!active && mode == TopLevelView::SubnetworkClustering
		               && stage2Running);
		tb->setInert(active || blocked, true);

		if (!blocked)
		{
			tb->setReturnJob([this, mode]()
			{
				if (_topLevelView == mode)
				{
					return;
				}

				// same mechanism the back button uses (see setup()) - a
				// no-op if nothing is currently assembling.
				if (_cancelled)
				{
					_cancelled->store(true);
				}
				VagWindow::window()->requestProgressBarRemoval();

				_topLevelView = mode;
				showTopLevelView();
			});
		}

		addTempObject(tb);
		return tb;
	};

	_correlationMatrixButton = make_button("Correlation matrix",
	                                       TopLevelView::CorrelationMatrix,
	                                       0.38);
	_subnetworkClusteringButton = make_button("Subnetwork clustering",
	                                          TopLevelView::SubnetworkClustering,
	                                          0.58);
}

void ViewCorrelations::showTopLevelView()
{
	clearScreen();
	_correlationMatrixButton = nullptr;
	_subnetworkClusteringButton = nullptr;
	makeTopLevelViewToggle();

	if (_topLevelView == TopLevelView::CorrelationMatrix)
	{
		if (_matrixComplete)
		{
			// already fully assembled and gap-filled by a previous run -
			// just rebuild the display from it rather than re-running the
			// whole (potentially slow) assembly again.
			buildCorrelationMatrixUI();
		}
		else
		{
			viewAll();
		}
	}
	else
	{
		showSubnetworkClustering();
	}
}

void ViewCorrelations::showSubnetworkClustering()
{
	std::vector<Clique *> subs;
	for (Clique &sub : _clique->subdivisions())
	{
		if (sub.states())
		{
			subs.push_back(&sub);
		}
	}

	int n = (int)subs.size();

	// cancellable via the back/toggle buttons, same mechanism (and same
	// _cancelled member) viewAll()'s own assembly uses - see its comment.
	// The O(n^2) overlap loop below is what actually takes a long time for
	// a clique with many subdivisions, so it - and everything derived from
	// it - runs on a background DoJob with a progress bar, one tick per
	// outer i, rather than blocking this callback (a UI thread callback)
	// until it finishes.
	auto cancelled = std::make_shared<std::atomic<bool>>(false);
	_cancelled = cancelled;

	struct ClusterProgress : public Progressor {};
	ClusterProgress *progress = new ClusterProgress();

	auto cancelJob = [cancelled]()
	{
		cancelled->store(true);
	};

	VagWindow::window()->requestProgressBar(n, "Clustering subnetworks",
	                                        progress, cancelJob);

	Clique *clique = _clique;
	auto cluster = [this, clique, subs, n, cancelled, progress]()
	{
		Eigen::MatrixXi overlap = Eigen::MatrixXi::Zero(n, n);
		int maxOverlap = 0;

		// distance derived from shared nodes between two subnetworks' own
		// CertainStates::ptps() - not just each subnetwork's own core
		// probes, since ptps() already includes whatever peripheral nodes
		// that subnetwork's own analysis pulled in, which is what
		// actually makes this an accurate correlation between
		// subnetworks rather than just their nominal membership.
		for (int i = 0; i < n; i++)
		{
			if (cancelled->load())
			{
				break;
			}

			for (int j = i + 1; j < n; j++)
			{
				OpSet<ProbeTypePair> common = subs[i]->states()->ptps()
				.common_to_both(subs[j]->states()->ptps());

				int shared = (int)common.size();
				overlap(i, j) = shared;
				overlap(j, i) = shared;

				if (shared > maxOverlap)
				{
					maxOverlap = shared;
				}
			}

			progress->clickTicker();
		}

		progress->finishTicker();

		if (cancelled->load())
		{
			delete progress;
			return;
		}

		// exp(-overlap^2 / scale): 0 shared nodes -> distance 1 (the
		// maximum, same as any other unrelated pair); overlap rising
		// towards maxOverlap pulls distance down towards exp(-1) =~ 0.37,
		// a smooth, continuous falloff rather than the previous linear
		// "maxOverlap - overlap", which put every pair on an evenly-spaced
		// integer ladder regardless of how the actual overlap counts were
		// distributed (e.g. all-or-nothing when most pairs shared ~0
		// nodes). scale = maxOverlap^2 is a self-normalising bandwidth:
		// it's tied to this clique's own most-correlated pair rather than
		// a fixed constant, so the curve is meaningful whether overlaps
		// typically run to single digits or the hundreds.
		float scale = (float)maxOverlap * (float)maxOverlap;
		if (scale < 1.f)
		{
			scale = 1.f;
		}

		Eigen::MatrixXf dist = Eigen::MatrixXf::Zero(n, n);
		for (int i = 0; i < n; i++)
		{
			for (int j = 0; j < n; j++)
			{
				if (i == j)
				{
					continue;
				}

				float o = (float)overlap(i, j);
				dist(i, j) = expf(-(o * o) / scale);
			}
		}

		// sized by how many nodes each subnetwork's own CertainStates
		// spans - no obvious energy analogue at this level (unlike
		// showStateClustering()'s per-state Boltzmann weight); a
		// subnetwork touching more of the structure is the closest
		// stand-in for "prominent" here.
		std::vector<float> sizeWeights(n);
		for (int i = 0; i < n; i++)
		{
			sizeWeights[i] = (float)subs[i]->states()->ptps().size();
		}

		// flags every subnetwork whose ptps() (own core probes plus
		// whatever peripheral nodes its analysis pulled in - same set
		// used for the overlap distance above) includes the signal
		// currently watched via CommunicationChoice, so ClusterPlot can
		// draw it as a star instead of a plain dot.
		std::string watched = clique->watchedSignal();
		std::vector<bool> starred(n, false);
		if (watched.length())
		{
			for (int i = 0; i < n; i++)
			{
				for (const ProbeTypePair &ptp : subs[i]->states()->ptps())
				{
					if (ptp.first && ptp.first->desc() == watched)
					{
						starred[i] = true;
						break;
					}
				}
			}
		}

		// hands off to the main thread rather than touching this Scene's
		// Renderables directly from the background thread - same reason
		// viewAll()'s own assemble() job does.
		VagWindow::window()->addMainThreadJob(
		[this, dist, sizeWeights, starred, cancelled, progress]()
		{
			delete progress;

			if (cancelled->load())
			{
				return;
			}

			ClusterPlot *cp = new ClusterPlot(dist, sizeWeights, getModel(),
			                                  starred);
			placeClusterPlot(cp);
		});
	};

	new DoJob(cluster);
}

void ViewCorrelations::showCorrelationMatrix(Clique &clique)
{
	if (!clique.states()) return;
	const CertainStates &states = *clique.states();

	int np = states.ptps().size();
	float ydim = std::min(0.05, 0.5 / (float)np);
	float xdim = ydim * 0.6;

	auto make_align_to_grid = [xdim, ydim, this]()
	{
		float offx = 0.5;
		float offy = 0.4;

		return [this, xdim, ydim, offx, offy](Renderable *r, int m, int n)
		{
			Renderable::Alignment align = 
			Renderable::Alignment
			(Renderable::Alignment::Centre | Renderable::Alignment::Middle);

			if (n == -1)
			{
				glm::mat4x4 base = glm::mat3x3(1.f);
				base[1] /= Window::aspect();
				glm::mat3x3 rot;
				rot = glm::mat3x3(glm::rotate(base, (float)deg2rad(-45),
				                              glm::vec3(0., 0., -1.)));
				r->rotateRoundCentre(rot);
				align = Renderable::Alignment
				(Renderable::Alignment::Left | Renderable::Alignment::Bottom);
			}
			else if (m == -1)
			{
				align = Renderable::Alignment(Renderable::Alignment::Right);
			}
			else
			{
				glm::mat3x3 rot = glm::mat3x3(1.f);
				rot[1] *= -1;
				r->rotateRoundCentre(rot);

			}
			m++;
			n++;

			r->setArbitrary(offx + m * xdim, offy + n * ydim, align);
			addTempObject(r);
		};
	};
	
	auto align_to_grid = make_align_to_grid();
	
	std::cout << "Number of probes: " << states.ptps().size() << std::endl;
	
	// states.ptps() is a set now (deterministic row order - see
	// CertainStates), so no operator[]; track position with a counter
	// while iterating instead, which gives identical grid positions to
	// the old positional-index loop.
	int p = 0;
	for (const ProbeTypePair &ptp : states.ptps())
	{
		std::string desc = ptp.first->desc();
		std::string type = ptp.second ==
		hnet::Types::BondType ? " bonding" : " exists";

		// states.ptps() reaches beyond clique.probes() (the subnetwork's
		// own core set Subdivide originally grew) - CertainStates/
		// ExhaustiveSearch's own wider-context lookup pulls in extra
		// peripheral probes it needed to resolve existence, which
		// otherwise look identical to the core set in this grid. Thick
		// font marks a probe that was actually part of the original
		// subnetwork; thin marks one dragged in only for that wider
		// resolution.
		Font::Type font = clique.probes().count(ptp.first) ?
		Font::Thick : Font::Thin;

		for (int i = 0; i < 2; i++)
		{
			Text *t = new Text(desc + type, font);
			t->resize(0.3);
			align_to_grid(t, (i == 0 ? -1 : p), (i == 0 ? p : -1));
		}
		p++;
	}

	// computed once for this run, not cached on states itself - score(i)
	// is a live callback that can change if enabled energy terms change
	// between separate runs, so this must be fresh every call.
	// probsForLocalAve() computes score(i) once per state and derives
	// both ave and probs from that same pass, instead of average_score()
	// and probsForAve() each calling it separately.
	float ave = 0;
	std::vector<float> probs = states.probsForLocalAve(ave);

	int m = 0;
	for (const ProbeTypePair &left : states.ptps())
	{
		int n = 0;
		for (const ProbeTypePair &right : states.ptps())
		{
			ProbeCorrelation corr = states.correlate(left, right, probs, true);

			// corr (and corr.mat with it) is loop-local and would
			// otherwise be gone by the time this MatrixPlot - added to
			// the grid below and outliving this iteration - next reads
			// from it. _subnetworkMats keeps a copy alive for as long as
			// the plot referencing it lives.
			_subnetworkMats.push_back(corr.mat);
			MatrixPlot *mp = new MatrixPlot(_subnetworkMats.back());
			mp->resize(xdim * 3);
			if (corr.mat.rows() == 2 && corr.mat.cols() == 2)
			{
				mp->resize(0.667);
			}
			align_to_grid(mp, m, n);
			n++;
		}
		m++;
	}
}

