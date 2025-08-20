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

#include <vagabond/gui/elements/AskForText.h>
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

	auto click = [this, scene, lg](Clique *clique)
	{
		return [this, clique, scene, lg](bool left)
		{
			if (left)
			{
				scene->selectProbes(clique->probes());
				return;
			}

			OpSet<Probe *> probes = clique->probes();
			
			auto change_name = [clique, scene]()
			{
				AskForText *aft = new AskForText(scene, "New clique name:",
				                                 "tag", scene);
				aft->setReturnJob
				([clique](std::string text)
				 {
					clique->setName(text);
				 });
				
				scene->setModal(aft);
			};
			
			auto analyse_bonds = [this, clique, scene]()
			{
				scene->deselect();
				scene->selectProbes(clique->probes());
				scene->completeResidues(true);
				scene->arrangeFigure();
				scene->setActive(clique);
				_kill();
			};
			
			Menu *menu = new Menu(scene);
			menu->addOption("rename", change_name);
			menu->addOption("analyse", analyse_bonds);

			float x = 0.5; float y = 0.5;
			if (_gl)
			{
				_gl->getFractionalPos(x, y);
			}

			menu->setup(x, y);
			scene->setModal(menu);
		};
	};

	auto handle_clique = [this, click, add_clique]
	(const OpSet<Probe *> &probes)
	{
		Clique *clique = _network.newClique(probes);
		clique->setHandleBeingChosen(click(clique));
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
			clique.setHandleBeingChosen(click(&clique));
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
