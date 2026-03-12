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

#include <vagabond/gui/elements/TextButton.h>
#include <vagabond/gui/elements/list/LineGroup.h>
#include <vagabond/gui/elements/list/ItemLine.h>
#include <vagabond/gui/elements/ScrollBox.h>
#include <vagabond/gui/elements/Menu.h>
#include <vagabond/core/protonic/Clique.h>
#include <vagabond/core/protonic/Network.h>
#include <vagabond/core/protonic/CliqueFinder.h>
#include <vagabond/gui/ProtonNetworkView.h>
#include <vagabond/utils/DoJob.h>
#include "CliqueView.h"

CliqueView::CliqueView(ProtonNetworkView *scene, Network &network, 
                       const OpSet<Probe *> &probes) 
: Image("assets/images/box.png"), _network(network), _scene(scene)
{
	clearVertices();
	addQuad();
	float width = 0.3;
	rotateByMatrix({width, 0, 0, 0.0, 0.8, 0.0, 0.0, 0.0, 1.f});
	setCentre(1 - (width / 2) - 0.05, 0.55);
	setupConstants();
	
	_parent.setDisplayName("All cliques");
	_ambiguous.setDisplayName("Ambiguous assignment");
	_certain.setDisplayName("Fully resolved");
	_wet.setDisplayName("Wet");
	_dry.setDisplayName("Dry");

	_parent.addItem(&_ambiguous);
	_ambiguous.addItem(&_wet);
	_ambiguous.addItem(&_dry);
	_parent.addItem(&_certain);

	LineGroup *lg = new LineGroup(&_parent, _scene);
	lg->setLeft(0.65, 0.23);

	ScrollBox *sb = new ScrollBox();
	sb->setContent(lg);
	sb->setBounds(glm::vec4(0.23, 0.65, 0.9, 0.9));
	addObject(sb);
	
	auto add_clique = [this](Clique *clique)
	{
		return [this, clique]()
		{
			clique->setDisplayName(clique->name());
			if (clique->is_certain())
			{
				_certain.addItem(clique);
			}
			else if (clique->num_waters() > 0)
			{
				_wet.addItem(clique);
			}
			else
			{
				_dry.addItem(clique);
			}
		};
	};

	auto handle_clique = [this, scene, lg, add_clique]
	(const OpSet<Probe *> &probes)
	{
		Clique *clique = _network.newClique(probes);
		
		auto click = [this, clique, scene, lg](bool left)
		{
			if (left)
			{
				scene->selectProbes(clique->probes());
				return;
			}
			
			bool has_interest = false;
			bool has_disinterest = false;
			
			OpSet<Probe *> probes = clique->probes();
			
			for (Probe *const &pr : probes)
			{
				(_interesting.count(pr) ? 
				 has_interest : has_disinterest) = true;
			}
			
			auto add_or_subtract = [this, probes, clique, lg](bool add)
			{
				return [this, add, probes, clique, lg]()
				{
					add ?  _interesting += probes : _interesting -= probes;
					if (add)
					{
						lg->display(clique)->setColour(0.4, 0.2, 0.4);
						_interesting += probes;
					}
					else
					{
						lg->display(clique)->setColour(0.0f, 0.0f, 0.0f);
						_interesting -= probes;
					}
				};
			};
			
			Menu *menu = new Menu(scene);
			if (has_disinterest)
			{
				menu->addOption("mark as interesting", 
				                add_or_subtract(true));
			}
			
			if (has_interest)
			{
				menu->addOption("mark as uninteresting", 
				                add_or_subtract(false));
			}
			
			float x = 0.5; float y = 0.5;
			if (_gl)
			{
				_gl->getFractionalPos(x, y);
			}

			menu->setup(x, y);
			scene->setModal(menu);
		};
		
		clique->setHandleBeingChosen(click);
		
		addMainThreadJob(add_clique(clique));
	};

	auto find_cliques = [probes, handle_clique, sb, lg, this]()
	{
		CliqueFinder finder{};
		finder.completeAndChop(probes, handle_clique);
		
		addMainThreadJob([sb, lg]()
		{
			lg->refreshGroups();
			sb->addSliderIfNeeded();
		});
	};
	
	if (_network.cliques().size())
	{
		for (Clique &clique : _network.cliques())
		{
			std::cout << "adding clique: " << clique.name() << std::endl;
			add_clique(&clique)();
		}
	}
	else
	{
		new DoJob(find_cliques);
	}
	
}

void CliqueView::setupCloseButton()
{
	TextButton *b = new TextButton("x", nullptr);
	b->setCentre(0.93, 0.2);
	
	auto kill_view = [this]()
	{
		if (!_kill)
		{
			std::cout << "Problem: Immortal view" << std::endl;
			return;
		}

		_kill();
	};
	
	b->setReturnJob(kill_view);
	addObject(b);
}

void CliqueView::setupConstants()
{
	setupCloseButton();
}
