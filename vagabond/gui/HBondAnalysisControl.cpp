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
#include <vagabond/gui/elements/AskYesNo.h>
#include <vagabond/gui/elements/TextButton.h>
#include <vagabond/gui/elements/ChooseRange.h>
#include <vagabond/gui/elements/ImageButton.h>
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

		auto add_subdiv_summary = [this]()
		{
			const std::list<Clique> &subs = _clique->subdivisions();
			if (subs.size() == 0)
			{
				return;
			}
			int sum = 0;
			for (const Clique &sub : subs)
			{
				sum += sub.probes().size();
			}
			float ave = sum / (float)subs.size();

			std::string summary = std::to_string(subs.size()) + " groups of \n"\
			"average " + f_to_str(ave, 1) + " nodes";

			Text *num = new Text(summary);
			num->resize(0.6);
			num->setCentre(0.18, 0.7);
			addTempObject(num);
		};
		
		auto delete_subdiv_button = [this]()
		{
			auto delete_subdivs = [this]()
			{
				_clique->setSubdivisions({});
				refresh();
			};

			if (_clique->subdivisions().size() && !searchIsRunning())
			{
				ImageButton *ib = new ImageButton("assets/images/cross.png", this);
				ib->resize(0.06);
				ib->setLeft(0.25, 0.7);
				ib->setReturnJob(delete_subdivs);
				addTempObject(ib);
			}
		};

		_refreshes.push_back(enable_subdivide_button);
		_refreshes.push_back(add_subdiv_summary);
		_refreshes.push_back(delete_subdiv_button);
	}
	
	auto exhaustive_search = [this]()
	{
		auto cancelled = std::make_shared<std::atomic<bool>>(false);
		auto running = std::make_shared<std::atomic<bool>>(true);

		// kept on the Clique itself, not a local/member of this Scene -
		// start_subdivisions() calls back() below, and this view may be
		// destroyed and freshly reconstructed by the time the user
		// returns to this clique, long before the search actually
		// finishes on its worker thread.
		_clique->searchRunning() = running;

		SearchAll *search = new SearchAll(_clique, _network);
		search->setCancelFlag(cancelled);
		search->setRunningFlag(running);

		Clique *clique = _clique;
		Network *network = &_network;

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

		// matches exactly what SearchAll::run() will call clickTicker()
		// for - it skips subdivisions that already have cached states
		// (e.g. left over from an earlier, cancelled run), so counting
		// every subdivision here would leave the bar short of its own
		// max on a re-run and never look like it finishes.
		int ticks = 0;
		for (const Clique &sub : _clique->subdivisions())
		{
			if (!(sub.states() && sub.states()->state_count()))
			{
				ticks++;
			}
		}

		auto cancelJob = [cancelled]()
		{
			cancelled->store(true);
		};

		VagWindow::window()->requestProgressBar(ticks, "Searching sub-networks",
		                                        nullptr, cancelJob);
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

		auto add_num_groups = [this]()
		{
			int num = _clique->allCommsNames().size();
			if (num > 0)
			{
				std::string str = (std::to_string(num) + " group" +
				                   (num == 1 ? "" : "s"));
				Text *num = new Text(str);
				num->setCentre(0.64, 0.86);
				num->resize(0.6);
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
