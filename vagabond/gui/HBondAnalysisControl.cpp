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
#include "ViewCorrelations.h"
#include "HBondAnalysisControl.h"
#include "CommunicationChoice.h"
#include <vagabond/gui/elements/AskYesNo.h>
#include <vagabond/gui/elements/TextButton.h>
#include <vagabond/core/protonic/Subdivide.h>
#include <vagabond/core/protonic/SearchAll.h>
#include <vagabond/core/protonic/Clique.h>
#include <vagabond/utils/FileReader.h>
#include <vagabond/utils/DoJob.h>

HBondAnalysisControl::HBondAnalysisControl(Scene *prev, Clique *clique,
                                           Network &network)
: Scene(prev), _clique(clique), _network(network)
{

}

void HBondAnalysisControl::setup()
{
	float left = 0.15;
	float right = 0.85;
	addTitle("Analysis overview");

	{
		TextButton *tb = new TextButton("Choose communication groups", this);
		tb->setLeft(left, 0.3);
		tb->setReturnJob([this]()
		{
			CommunicationChoice *cc = new CommunicationChoice(this, _clique);
			cc->show();
		});
		
		auto add_num_groups = [this, right]()
		{
			int num = _clique->allCommsNames().size();
			if (num > 0)
			{
				std::string str = (std::to_string(num) + " group" + 
				                   (num == 1 ? "" : "s"));
				Text *num = new Text(str);
				num->setRight(right, 0.3);
				addTempObject(num);
			}
		};

		_refreshes.push_back(add_num_groups);
		addObject(tb);
	}
	
	auto subdivide_with_defaults = [this]()
	{
		Subdivide sd(_clique);
		refresh();
	};
	
	auto ask_to_subdivide = [this, subdivide_with_defaults]()
	{
		AskYesNo *ayn = new AskYesNo(this, "Subdivide network using defaults?");
		ayn->addJob("yes", subdivide_with_defaults);
		setModal(ayn);
	};

	{
		TextButton *tb = new TextButton("Subdivide network", this);
		tb->setLeft(left, 0.4);
		tb->setReturnJob(ask_to_subdivide);
		addObject(tb);
		
		auto add_subdiv_summary = [this, right]()
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

			std::string summary = std::to_string(subs.size()) + " groups of "\
			"average " + f_to_str(ave, 1) + " nodes";

			Text *num = new Text(summary);
			num->setRight(right, 0.4);
			addTempObject(num);
		};
		
		_refreshes.push_back(add_subdiv_summary);
	}
	
	auto start_subdivisions = [this]()
	{
		SearchAll *search = new SearchAll(_clique, _network);
		new DoJob([search]()
		{
			search->run();
			delete search;
		});
		back();
		int ticks = _clique->subdivisions().size();
		VagWindow::window()->requestProgressBar(ticks, "Searching sub-networks");
	};
	
	{
		TextButton *tb = new TextButton("Exhaustive search", this);
		tb->setLeft(left, 0.5);
		tb->setReturnJob(start_subdivisions);
		addObject(tb);
		
		auto view_correlations = [this]()
		{
			ViewCorrelations *vc = new ViewCorrelations(this, _clique);
			vc->show();
		};

		auto enable_with_divisions = [this, tb]()
		{
			const std::list<Clique> &subs = _clique->subdivisions();
			if (subs.size() == 0)
			{
				tb->setInert(true, true);
			}
			else
			{
				tb->setInert(false, true);
			}
		};
		
		_refreshes.push_back(enable_with_divisions);

		auto show_view_results = [this, view_correlations, right]()
		{
			const std::list<Clique> &subs = _clique->subdivisions();
			bool has_results = false;
			for (const Clique &sub : subs)
			{
				if (sub.results().size())
				{
					has_results = true; break;
				}
			}

			TextButton *tb = new TextButton("View correlations", this);
			tb->setRight(right, 0.5);
			tb->setReturnJob(view_correlations);
			addTempObject(tb);
		};
		
		_refreshes.push_back(show_view_results);
	}

	{
		TextButton *tb = new TextButton("Prepare groups for path", this);
		tb->setLeft(left, 0.6);
		addObject(tb);
		
		auto enable_for_groups = [this, tb]()
		{
			int num = _clique->allCommsNames().size();
			if (num >= 2)
			{
				tb->setInert(false, true);
			}
			else if (num >= 1)
			{
				tb->addAltTag("Requires one more communication group "\
				              "to be defined");
				tb->setInert(true, true);
			}
			else if (num >= 0)
			{
				tb->addAltTag("Requires definition of communication groups");
				tb->setInert(true, true);
			}
		};
		
		_refreshes.push_back(enable_for_groups);
	}

	{
		TextButton *tb = new TextButton("Best path search", this);
		tb->setLeft(left, 0.7);
		tb->setInert(true, true);
		addObject(tb);
	}
	
	refresh();
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
