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
#include <vagabond/gui/elements/ImageButton.h>
#include <vagabond/gui/elements/Text.h>
#include <vagabond/gui/elements/InfoModal.h>
#include <vagabond/gui/elements/list/LineGroup.h>
#include <vagabond/gui/elements/list/ItemLine.h>
#include <vagabond/gui/elements/ScrollBox.h>
#include <vagabond/gui/elements/Menu.h>
#include <vagabond/core/protonic/Clique.h>
#include <vagabond/core/protonic/Network.h>
#include <vagabond/gui/ProtonNetworkView.h>
#include <vagabond/gui/HBondAnalysisControl.h>
#include <vagabond/utils/DoJob.h>
#include "CliqueView.h"
#include <SDL2/SDL_clipboard.h>

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
			insert_clique(_parent, clique);
		};
	};
	
	auto click = [this, reinsert, scene, lg](Clique *clique)
	{
		return [this, reinsert, clique, scene](bool left)
		{
			if (left)
			{
				if (_scene->controlPressed())
				{
					scene->selectProbes(clique->probes(), false);
				}
				else if (!_scene->shiftPressed())
				{
					scene->deselect();
				}
				if (!_scene->controlPressed())
				{
					scene->selectProbes(clique->probes());
				}
				if (!_scene->shiftPressed() && clique->hasCentroid())
				{
					scene->shiftToCentre(clique->centroid(), 0);
				}
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
				// 3D is the default view for an active clique now - 2D is
				// opt-in via the "Switch to 2D" menu option.
				scene->deselect();
				scene->selectProbes(clique->probes());
				scene->completeResidues(true);
				scene->setActive(clique);

				HBondAnalysisControl *hbac =
				new HBondAnalysisControl(scene, clique, _network);
				hbac->show();

				_kill();
			};

			auto delete_clique = [this, clique]()
			{
				// unlink from the display groups and this view's cache
				// while the pointer is still valid, then erase the Clique
				// itself from the Network - that also pushes the removal
				// through to Model so it is gone from the next json save.
				clique->removeSelf(true);

				auto it = std::find(_cliques.begin(), _cliques.end(), clique);
				if (it != _cliques.end())
				{
					_cliques.erase(it);
				}

				_network.removeClique(clique);

				_lg->refreshGroups();
			};

			Menu *menu = new Menu(scene);
			menu->addOption("rename", change_name);
			menu->addOption("analyse", analyse_bonds);

			if (clique->planText().size())
			{
				auto copy_plan = [clique]()
				{
					SDL_SetClipboardText(clique->planText().c_str());
				};
				menu->addOption("copy plan to clipboard", copy_plan);
			}

			menu->addOption("delete", delete_clique);

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

	wireDescendants(clique);
}

void CliqueView::rewireSubdivisions()
{
	for (Clique *clique : _cliques)
	{
		wireDescendants(clique);
	}
}

void CliqueView::wireDescendants(Item *item)
{
	for (Item *child : item->items())
	{
		Clique *sub = static_cast<Clique *>(child);
		wireSubdivision(sub);
		wireDescendants(sub);
	}
}

void CliqueView::wireSubdivision(Clique *sub)
{
	ProtonNetworkView *scene = _scene;

	auto select_and_centre = [this, scene](Clique *clique)
	{
		if (_scene->controlPressed())
		{
			scene->selectProbes(clique->probes(), false);
		}
		else if (!_scene->shiftPressed())
		{
			scene->deselect();
		}
		if (!_scene->controlPressed())
		{
			scene->selectProbes(clique->probes());
		}
		if (!_scene->shiftPressed())
		{
			scene->shiftToCentre(clique->centroid(), 0);
		}
	};

	auto analyse_sub = [this, sub, scene]()
	{
		scene->deselect();
		scene->selectProbes(sub->probes());
		scene->completeResidues(true);
		scene->setActive(sub);
		_kill();
	};

	sub->setSelectJob([this, select_and_centre, sub, scene, analyse_sub]
	                  (bool left)
	{
		if (left)
		{
			select_and_centre(sub);
			return;
		}

		Menu *menu = new Menu(scene);
		menu->addOption("analyse", analyse_sub);

		float x = 0.5; float y = 0.5;
		if (_gl)
		{
			_gl->getFractionalPos(x, y);
		}

		menu->setup(x, y);
		scene->setModal(menu);
	});
}

CliqueView::CliqueView(ProtonNetworkView *scene, Network &network)
: Image("assets/images/box.png"), _network(network), _scene(scene)
{
	clearVertices();
	addQuad();
	float width = 0.3;
	rotateByMatrix({width, 0, 0, 0.0, 0.8, 0.0, 0.0, 0.0, 1.f});
	setCentre(1 - (width / 2) - 0.05, 0.55);
	setupConstants();

	// named before LineGroup/ItemLine ever get constructed below, not
	// after - ItemLine::replaceContent() builds its own Text straight
	// from displayName() the moment it first runs (inside LineGroup's own
	// constructor), and _unitHeight/every indent derived from it comes
	// from that Text's own measured height - an Item still unnamed at
	// that point got a zero-height, zero-width placeholder that later
	// updates (see setDisplayName()'s own triggerResponse()) haven't
	// reliably recovered from. Naming first avoids that first pass ever
	// happening at all, rather than needing it to self-correct.
	_parent.setDisplayName("All cliques");

	LineGroup *lg = new LineGroup(&_parent, _scene);
	// 0.05 below the ScrollBox's own top clip bound, not the same value -
	// matching ViewCorrelations::makeList() and CommunicationAnalysis's
	// own ScrollBox setup, both of which already give their content a
	// margin here. Without it, the first row's own text (centred on its
	// own top-aligned position, so extending slightly above it) got
	// clipped by the scrollbox's clip shader before it could ever render.
	lg->setLeft(0.68, 0.28);
	_lg = lg;

	ScrollBox *sb = new ScrollBox();
	sb->setContent(lg);
	lg->setScrollBox(sb);
	sb->setBounds(glm::vec4(0.23, 0.65, 0.9, 0.92));
	addObject(sb);

	auto add_clique = [this](Clique *clique)
	{
		return [this, clique]()
		{
			insertClique(clique);
		};
	};

	// no automatic clique finder any more - cliques only ever come from
	// whatever was already saved on network, or from the user's own
	// "make new clique" (ProtonNetworkView::interactedWithNothing()) from
	// here on.
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

	addHelpButton();
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

void CliqueView::addHelpButton()
{
	std::string text =
	"To create a new clique, shift + drag and select atoms with the mouse,\n"\
	"or right click empty space and click 'select using plan'.\n\n"\
	"To examine the hydrogen-bonding network behaviour of a clique,\n"\
	"right click the clique's name in the list and choose 'analyse'.";

	// straight to the question-mark icon, no inline text first - same
	// idiom as Modal::setHelpText() (a small icon that opens the text in
	// an InfoModal on click), replicated directly since this view isn't
	// a Modal itself.
	ImageButton *ib = new ImageButton("assets/images/question_mark.png",
	                                  nullptr);
	ib->resize(0.04);
	ib->setRight(0.69, 0.22);
	ib->setReturnJob([this, text]()
	{
		InfoModal *info = new InfoModal(_scene, text);
		_scene->setModal(info, false);
	});
	addObject(ib);
	_helpButton = ib;
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
