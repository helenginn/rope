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

void CliqueView::insertClique(Clique *clique)
{
	ProtonNetworkView *scene = _scene;
	LineGroup *lg = _lg;

	auto reinsert = [this](Clique *clique)
	{
		Item *parent = clique->parent();
		Item *lastWithName = nullptr;
		for (Item *const &child : parent->items())
		{
			Clique *cl = static_cast<Clique *>(child);
			bool has_name = cl->hasCustomName();
			
			if (has_name)
			{
				lastWithName = cl;
			}
			else
			{
				break;
			}
		}

		if (clique == lastWithName)
		{
			return;
		}

		clique->removeSelf(true);
		parent->addItemAfter(clique, lastWithName);
	};
	
	auto insert_clique = [this, reinsert](Item &parent, Clique *clique)
	{
		if (clique->parent() == &parent)
		{
			reinsert(clique);
			return;
		}
		parent.addItem(clique);
		reinsert(clique);
		_cliques.push_back(clique);
	};
	
	auto add_clique = [this, insert_clique](Clique *clique)
	{
		return [this, insert_clique, clique]()
		{
			clique->setDisplayName(clique->name());
			clique->removeSelf(true);
			if (clique->is_certain())
			{
				insert_clique(_certain, clique);
			}
			else if (clique->num_waters() > 0)
			{
				insert_clique(_wet, clique);
			}
			else
			{
				insert_clique(_dry, clique);
			}
		};
	};
	
	auto click = [this, reinsert, scene, lg](Clique *clique)
	{
		return [this, reinsert, clique, scene](bool left)
		{
			if (_combine == clique)
			{
				_combine = nullptr;
				return;
			}
			if (left && !_combine)
			{
				scene->deselect();
				scene->selectProbes(clique->probes());
				scene->shiftToCentre(clique->centroid(), 0);
				return;
			}
			else if (left && _combine)
			{
				Clique *combined = _network.newClique({});
				combined->add_clique(*_combine);
				combined->add_clique(*clique);
				std::string new_name = _combine->displayName();
				new_name += " + " + clique->displayName();
				insertClique(combined);
				combined->setDisplayName(new_name);
				
				auto it = std::find(_cliques.begin(), _cliques.end(), clique);
				if (it != _cliques.end())
				{
					_cliques.erase(it);
				}

				_combine = nullptr;
				return;
			}

			OpSet<Probe *> probes = clique->probes();
			
			auto change_name = [clique, reinsert, scene]()
			{
				AskForText *aft = new AskForText(scene, "New clique name:",
				                                 "tag", scene);
				aft->setReturnJob
				([reinsert, clique](std::string text)
				 {
					clique->setName(text);
					reinsert(clique);
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
			if (!_combine)
			{
				menu->addOption("rename", change_name);
				menu->addOption("analyse", analyse_bonds);
				menu->addOption("combine with...", [this, clique]() 
				                { _combine = clique; });
			}
			else
			{
				menu->addOption("cancel combine", [this]() 
				                { _combine = nullptr; });
			}

			float x = 0.5; float y = 0.5;
			if (_gl)
			{
				_gl->getFractionalPos(x, y);
			}

			menu->setup(x, y);
			scene->setModal(menu);
		};
	};

	add_clique(clique)();
	clique->setSelectJob(click(clique));
}

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

	_parent.addItem(&_ambiguous);
	_ambiguous.addItem(&_wet);
	_ambiguous.addItem(&_dry);
	_parent.addItem(&_certain);

	LineGroup *lg = new LineGroup(&_parent, _scene);
	lg->setLeft(0.65, 0.23);
	_lg = lg;

	ScrollBox *sb = new ScrollBox();
	sb->setContent(lg);
	lg->setScrollBox(sb);
	sb->setBounds(glm::vec4(0.23, 0.65, 0.9, 0.9));
	addObject(sb);
	
	_parent.setDisplayName("All cliques");
	_ambiguous.setDisplayName("Ambiguous assignment");
	_certain.setDisplayName("Fully resolved");
	_wet.setDisplayName("Wet");
	_dry.setDisplayName("Dry");

	auto add_clique = [this](Clique *clique)
	{
		return [this, clique]()
		{
			insertClique(clique);
		};
	};

	auto handle_clique = [this, add_clique]
	(const OpSet<Probe *> &probes)
	{
		Clique *clique = _network.newClique(probes);
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
			add_clique(&clique)();
		}

		addMainThreadJob([sb, lg]()
		{
			lg->refreshGroups();
			sb->addSliderIfNeeded();
		});
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

void CliqueView::highlightCliquesWith(const OpSet<Probe *> &probes)
{
	for (Clique *clique : _cliques)
	{
		if (clique->probes().common_to_both(probes).size())
		{
			clique->setTextColour({0.4, 0.0, 0.4});
		}
		else
		{
			clique->setTextColour({0.0, 0.0, 0.0});
		}
	}

	_lg->refreshGroups();
}

void setActive(Clique *clique);
