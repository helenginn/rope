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

#include "VagWindow.h"
#include "OccupanciesView.h"
#include "ViewCorrelations.h"
#include "HBondAnalysisControl.h"
#include "CommunicationChoice.h"
#include "SubdivisionRunDetails.h"
#include <vagabond/gui/elements/AskYesNo.h>
#include <vagabond/gui/elements/TextButton.h>
#include <vagabond/gui/elements/ChooseRange.h>
#include <vagabond/gui/elements/ImageButton.h>
#include <vagabond/gui/elements/TickBoxes.h>
#include <vagabond/gui/elements/Window.h>
#include <vagabond/core/protonic/Subdivide.h>
#include <vagabond/core/protonic/SearchAll.h>
#include <vagabond/core/protonic/Clique.h>
#include <vagabond/core/protonic/CertainStates.h>
#include <vagabond/utils/FileReader.h>
#include <vagabond/utils/DoJob.h>
#include <atomic>
#include <memory>

HBondAnalysisControl::HBondAnalysisControl(Scene *prev, Clique *clique,
                                           Network &network)
: Scene(prev), _clique(clique), _network(network)
{

}

void HBondAnalysisControl::setup()
{
	addTitle("Analysis overview");

	auto subdivide_with_values = [this](int max, int samples)
	{
		return [this, max, samples]()
		{
			Subdivide sd(_clique, max);
			sd.subdivide(samples);
			refresh();
		};
	};

	auto dont_subdivide = [this]()
	{
		Subdivide sd(_clique, INT_MAX);
		sd.one();
		refresh();
	};

	auto ask_for_samples = [this, subdivide_with_values](int guideSize)
	{
		return [this, subdivide_with_values, guideSize]()
		{
			ChooseRange *cr = new ChooseRange(this, "How many samples "\
			                                  "per node?", "", this);
			cr->setDefault(3, 3);
			cr->setRange(1, 20, 19);
			cr->setReturn([subdivide_with_values, guideSize]
			              (float, float samples)
			{
				subdivide_with_values(guideSize, lrint(samples))();
			});

			setModal(cr);
		};
	};

	auto ask_for_guide_size = [this, ask_for_samples]()
	{
		ChooseRange *cr = new ChooseRange(this, "Set guide size for "\
		                                  "hydrogen bond subnetwork", "",
		                                  this);
		cr->setDefault(12, 12);
		cr->setRange(2, 50, 48);
		cr->setReturn([this, ask_for_samples](float, float max)
		{
			int guideSize = lrint(max);

			// ChooseRange::buttonPressed() calls this return callback
			// and then unconditionally hide()s - which clears every
			// modal on the scene (Modal::hide() -> Scene::removeModals()),
			// not just this one. Opening the next ChooseRange here
			// directly would show it and then have it wiped out by that
			// same hide() call, all within this one call stack.
			// Deferring to the next main-thread job runs it after that
			// cleanup has finished.
			addMainThreadJob([ask_for_samples, guideSize]()
			{
				ask_for_samples(guideSize)();
			});
		});

		setModal(cr);
	};

	auto ask_to_brute_force = [this, ask_for_guide_size, dont_subdivide]()
	{
		AskYesNo *ayn = new AskYesNo(this, "Do entire network by brute force?");
		ayn->addJob("yes", dont_subdivide);
		ayn->addJob("no", ask_for_guide_size);
		setModal(ayn);
	};

	auto ask_to_subdivide = [this, ask_to_brute_force, subdivide_with_values]()
	{
		AskYesNo *ayn = new AskYesNo(this, "Subdivide network using defaults?");
		ayn->addJob("yes", subdivide_with_values(12, 3));
		ayn->addJob("no", ask_to_brute_force);
		setModal(ayn);
	};

	{
		ImageButton *ib = new ImageButton("assets/images/subdivide.png",
		                                  nullptr);
		ib->resize(0.24);
		ib->setCentre(0.18, 0.45);
		ib->setReturnJob(ask_to_subdivide);
		addObject(ib);

		TextButton *text = new TextButton("Subdivide network", nullptr);
		text->resize(0.7);
		text->setCentre(0.18, 0.6);
		text->setReturnJob(ask_to_subdivide);
		addObject(text);

		auto enable_subdivide_button = [this, text, ib]()
		{
			text->setInert(searchIsRunning(), true);
			ib->setInert(searchIsRunning(), true);
		};

		auto add_subdivision_run_rows = [this]()
		{
			std::list<SubdivisionRun> &runs = _clique->subdivisionRuns();
			if (runs.size() == 0)
			{
				return;
			}

			bool inert = searchIsRunning();
			float y = 0.68f;

			for (SubdivisionRun &run : runs)
			{
				SubdivisionRun *ptr = &run;

				TickBoxes *tick = new TickBoxes(this, this);
				tick->addOption("", "active", run.active);
				tick->arrange(0.06, y, 0.10, y + 0.035);
				tick->setInert("active", inert);
				tick->setReturnJob([this, tick, ptr]()
				{
					bool nowTicked = tick->isTicked("active");
					_clique->setActiveSubdivisionRun(nowTicked ? ptr : nullptr);
					refresh();
				});
				addTempObject(tick);

				TextButton *name = new TextButton(run.displayName(), nullptr);
				name->resize(0.45);
				name->setLeft(0.12, y);
				name->setInert(inert, true);
				name->setReturnJob([this, ptr]()
				{
					SubdivisionRunDetails *details =
					new SubdivisionRunDetails(this, _clique, ptr,
					                          [this]() { refresh(); });
					setModal(details);
				});
				addTempObject(name);

				y += 0.05;
			}
		};

		_refreshes.push_back(enable_subdivide_button);
		_refreshes.push_back(add_subdivision_run_rows);
	}
	
	auto exhaustive_search = [this]()
	{
		auto cancelled = std::make_shared<std::atomic<bool>>(false);
		auto running = std::make_shared<std::atomic<bool>>(true);

		// kept on the Clique itself, not a local/member of this Scene -
		// start_subdivisions() calls back() below, and this view may be
		// destroyed and freshly reconstructed by the time the user
		// returns to this clique, long before the search actually
		// finishes on its worker thread. searchCancelled() likewise, so
		// ProtonNetworkView::cancelAnalysis() can request cancellation
		// of a search it didn't start (e.g. via the 3D view's right-click
		// menu, without ever having opened the progress bar itself).
		_clique->searchRunning() = running;
		_clique->searchCancelled() = cancelled;

		SearchAll *search = new SearchAll(_clique, _network);
		search->setCancelFlag(cancelled);
		search->setRunningFlag(running);

		Clique *clique = _clique;
		Network *network = &_network;

		// matches exactly what SearchAll::run() will call clickTicker()
		// for - every subdivision, unconditionally (it no longer skips
		// ones that already have cached states from an earlier run;
		// this drill re-searches everything every time it is clicked).
		int ticks = (int)_clique->subdivisions().size();

		auto cancelJob = [cancelled]()
		{
			cancelled->store(true);
		};

		// search itself is a Progressor - passed as caller here (instead
		// of nullptr) so its responder is registered synchronously, right
		// now, on this thread, before the DoJob below can possibly start
		// ticking. With caller == nullptr, that registration only happens
		// inside a deferred main-thread job (requestProgressBar's own),
		// so a fast-finishing search (ticks == 0, e.g. no subdivisions at
		// all) could call finishTicker() before the bar existed to hear
		// it - the "done" event would be silently lost, leaving a bar
		// that gets created afterward but never completes.
		//
		// skipped entirely when there's nothing to search (no
		// subdivisions at all) - ProgressBar::setMaxTicks(0) calls
		// finish() synchronously, which queues its own removal within
		// the same job-queue drain that creates the bar, so it would be
		// added and removed again before a single frame ever rendered it.
		if (ticks > 0)
		{
			VagWindow::window()->requestProgressBar(ticks,
			                                        "Searching sub-networks",
			                                        search, cancelJob);
		}

		new DoJob([search, clique, network]()
		{
			search->run();
			delete search;

			// reopen the overview once the search actually stops (whether
			// finished or cancelled) - on top of whatever's currently
			// showing, not necessarily the scene this was started from,
			// since the user may have navigated elsewhere while it ran.
			VagWindow::window()->addMainThreadJob([clique, network]()
			{
				Scene *current = Window::currentScene();
				HBondAnalysisControl *hbac =
				new HBondAnalysisControl(current, clique, *network);
				hbac->show();
			});
		});
		back();
	};

	// has_results()/comms_chosen() drive the enable-chain below: subdivide
	// -> exhaustive search -> (results exist) -> signals unlocked ->
	// (signals chosen) -> view correlations unlocked. Occupancies only
	// needs results, not chosen signals.
	auto has_results = [this]()
	{
		const std::list<Clique> &subs = _clique->subdivisions();
		for (const Clique &sub : subs)
		{
			if (sub.states() && sub.states()->state_count())
			{
				return true;
			}
		}
		return false;
	};

	auto comms_chosen = [this]()
	{
		return _clique->allCommsNames().size() > 0;
	};

	auto view_correlations = [this]()
	{
		ViewCorrelations *vc = new ViewCorrelations(this, _clique);
		vc->show();
	};

	auto view_occs = [this]()
	{
		OccupanciesView *ov = new OccupanciesView(this, _clique, _network);
		ov->show();
	};

	auto choose_signals = [this]()
	{
		CommunicationChoice *cc = new CommunicationChoice(this, _clique);
		cc->show();
	};

	{
		Image *i = new Image("assets/images/white_arrow.png");
		i->resize(0.10);
		i->setCentre(0.32, 0.45);
		addObject(i);
	}

	{
		Image *i = new Image("assets/images/white_arrow.png");
		i->resize(0.10);
		i->rotateAspectCorrected(-30);
		i->setCentre(0.54, 0.385);
		addObject(i);
	}

	{
		Image *i = new Image("assets/images/white_arrow.png");
		i->resize(0.10);
		i->rotateAspectCorrected(+30);
		i->setCentre(0.54, 0.585);
		addObject(i);
	}

	{
		Image *i = new Image("assets/images/white_arrow.png");
		i->resize(0.10);
		i->setCentre(0.75, 0.65);
		addObject(i);
	}

	{
		ImageButton *ib = new ImageButton("assets/images/drill.png", nullptr);
		ib->resize(0.24);
		ib->setCentre(0.4, 0.45);
		ib->setReturnJob(exhaustive_search);
		addObject(ib);

		TextButton *tb = new TextButton("Exhaustive search", nullptr);
		tb->resize(0.7);
		tb->setCentre(0.4, 0.6);
		tb->setReturnJob(exhaustive_search);
		addObject(tb);

		auto enable_with_divisions = [this, ib, tb]()
		{
			const std::list<Clique> &subs = _clique->subdivisions();
			// also disabled while already running - starting a second
			// SearchAll on the same clique would race the first one over
			// the same subdivisions list.
			if (subs.size() == 0 || searchIsRunning())
			{
				ib->setInert(true, true);
				tb->setInert(true, true);
			}
			else
			{
				ib->setInert(false, true);
				tb->setInert(false, true);
			}
		};

		_refreshes.push_back(enable_with_divisions);
	}

	{
		ImageButton *ib = new ImageButton("assets/images/signals.png",
		                                  nullptr);
		ib->resize(0.24);
		ib->setCentre(0.64, 0.65);
		ib->setReturnJob(choose_signals);
		addObject(ib);

		TextButton *tb = new TextButton("Choose signals", nullptr);
		tb->resize(0.7);
		tb->setCentre(0.64, 0.8);
		tb->setReturnJob(choose_signals);
		addObject(tb);

		auto enable_signals = [this, has_results, ib, tb]()
		{
			// also gated on !searchIsRunning(): SearchAll writes
			// clique.setStates(...) on each subdivision as it goes, on its
			// own background thread - reading subdivisions/states while
			// that's still in flight (e.g. from ViewCorrelations' own
			// background assembly) is a data race, worse the longer the
			// search runs, i.e. the bigger the clique.
			bool ok = has_results() && !searchIsRunning();
			ib->setInert(!ok, true);
			tb->setInert(!ok, true);
		};

		_refreshes.push_back(enable_signals);

		auto add_num_groups = [this, has_results]()
		{
			int num = _clique->allCommsNames().size();
			if (num > 0)
			{
				std::string str = (std::to_string(num) + " group" +
				                   (num == 1 ? "" : "s"));
				Text *num = new Text(str);
				num->setCentre(0.64, 0.86);
				num->resize(0.6);

				// Text isn't a Button, so it has no setInert() of its own -
				// same alpha values Button::setInert(!ok, true) applies,
				// so this tracks "Choose signals" (enable_signals above)
				// instead of staying opaque regardless of search state.
				bool ok = has_results() && !searchIsRunning();
				num->setAlpha(ok ? 0.f : -0.5f);

				addTempObject(num);
			}
		};

		_refreshes.push_back(add_num_groups);
	}

	{
		ImageButton *ib =
		new ImageButton("assets/images/communication.png", nullptr);
		ib->resize(0.24);
		ib->setCentre(0.86, 0.65);
		ib->setReturnJob(view_correlations);
		addObject(ib);

		TextButton *tb = new TextButton("View correlations", nullptr);
		tb->resize(0.7);
		tb->setCentre(0.86, 0.8);
		tb->setReturnJob(view_correlations);
		addObject(tb);

		auto enable_view_correlations = [this, has_results, comms_chosen,
		                                 ib, tb]()
		{
			bool ok = has_results() && comms_chosen() && !searchIsRunning();
			ib->setInert(!ok, true);
			tb->setInert(!ok, true);
		};

		_refreshes.push_back(enable_view_correlations);
	}

	{
		ImageButton *ib =
		new ImageButton("assets/images/occupancy_check.png", nullptr);
		ib->resize(0.24);
		ib->setCentre(0.64, 0.30);
		ib->setReturnJob(view_occs);
		addObject(ib);

		TextButton *tb = new TextButton("Occupancies", nullptr);
		tb->resize(0.7);
		tb->setCentre(0.64, 0.45);
		tb->setReturnJob(view_occs);
		addObject(tb);

		auto enable_occupancies = [this, has_results, ib, tb]()
		{
			bool ok = has_results() && !searchIsRunning();
			ib->setInert(!ok, true);
			tb->setInert(!ok, true);
		};

		_refreshes.push_back(enable_occupancies);
	}

	refresh();
}

bool HBondAnalysisControl::searchIsRunning()
{
	auto &flag = _clique->searchRunning();
	return flag && flag->load();
}

void HBondAnalysisControl::refresh()
{
	deleteTemps();

	std::cout << "refreshing" << std::endl;
	for (const std::function<void()> &refresh : _refreshes)
	{
		refresh();
	}
}
