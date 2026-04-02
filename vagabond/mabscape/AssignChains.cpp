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

#include "Mab.h"
#include "AssignChains.h"
#include <vagabond/core/Chain.h>
#include <vagabond/core/Model.h>
#include <vagabond/gui/elements/Menu.h>
#include <vagabond/gui/elements/Text.h>
#include <vagabond/gui/elements/Image.h>
#include <vagabond/core/EntityManager.h>
#include <vagabond/core/PolymerEntity.h>
#include <vagabond/gui/ModelTopologyView.h>
#include <vagabond/gui/elements/BadChoice.h>
#include <vagabond/gui/elements/AskForText.h>
#include <vagabond/core/SequenceComparison.h>

AssignChains::AssignChains(Scene *parent, 
                           Model &model, 
                           ColourMap &colours,
                           OpSet<std::string> &names)
: _model(model), _colours(colours), _names(names), _parent(parent)
{

}

std::function<void()> AssignChains::operator()()
{
	Scene *parent = _parent;
	Model *model = &_model;
	ColourMap *colours = &_colours;
	OpSet<std::string> *names = &_names;

	auto provide_colours = [model, colours](Chain *ch) -> glm::vec3
	{
		std::string entity = model->entityForChain(ch->id());
		if (entity.length() == 0)
		{
			return {0., 0., 0.};
		}
		else
		{
			return colours->colour_for(entity);
		}
	};

	auto assign_new_entity = [model, names, colours]
	(Chain *ch, ModelTopologyView *view)
	{
		return [ch, view, model, names, colours](std::string str)
		{
			try
			{
				PolymerEntity obj; obj.setName(str);
				obj.setSequence(ch->fullSequence()); 
				Environment::entityManager()->insertIfUnique(obj);
				model->setEntityForChain(ch->id(), str);
				model->housekeeping();
				colours->recalculate();
				*names += str;
				view->updateColours();
				view->updateLegend();
			}
			catch (const std::runtime_error &err)
			{
				BadChoice *bad = new BadChoice(view, err.what());
				view->setModal(bad);
			}
		};
	};

	auto assign_monomer = [model, colours, names]
	(ModelTopologyView *view, Chain *ch, const std::string &ent)
	{
		return [ent, ch, view, &model, names, colours]()
		{
			model->setEntityForChain(ch->id(), ent);
			model->housekeeping();
			colours->recalculate();
			*names += ent;
			view->updateColours();
			view->updateLegend();
		};
	};

	auto unassign_monomer = [model, colours]
	(ModelTopologyView *view, Chain *ch)
	{
		return [ch, view, colours, model]()
		{
			model->unassignChainEntity(ch->id());
			model->housekeeping();
			colours->recalculate();
			view->updateColours();
			view->updateLegend();
		};
	};

	auto make_ask_for_monomer = [assign_new_entity]
	(ModelTopologyView *view, Chain *ch)
	{
		return [assign_new_entity, ch, view]()
		{
			AskForText *aft = 
			new AskForText(view, "Name new monomer:", 
			               "", nullptr);
			aft->setReturnJob(assign_new_entity(ch, view));
			view->setModal(aft);
		};
	};

	auto click_chain = [make_ask_for_monomer, assign_monomer, 
	unassign_monomer, model]
	(ModelTopologyView *view, Chain *ch, const glm::vec3 &where)
	{
		EntityManager *m = Environment::entityManager();
		std::vector<Entity *> entities = m->entities();

		Menu *menu = new Menu(view);
		menu->addOption("assign as new protein",
		                make_ask_for_monomer(view, ch));
		for (Entity *const &ent : entities)
		{
			if (!ent->hasSequence())
			{
				continue;
			}

			Sequence *trial = 
			static_cast<PolymerEntity *>(ent)->sequence(); 

			SequenceComparison *sc = 
			new SequenceComparison(ch->fullSequence(), trial);
			                 
			float match = sc->match();
			
			std::string msg = "assign as " + ent->name();
			msg += " (" + f_to_str(match * 100, 1) + "% alignment)";

			menu->addOption(msg,
			                assign_monomer(view, ch, ent->name()));
		}
		if (model->entityForChain(ch->id()).length())
		{
			menu->addOption("unassign", unassign_monomer(view, ch));

		}
		menu->setup(where.x, where.y, 1);
		view->setModal(menu);
	};
	
	auto show_legend = [colours]()
	{
		EntityManager *m = Environment::entityManager();
		std::vector<Entity *> entities = m->entities();

		Box *box = new Box();
		float top = 0;
		for (Entity *const &ent : entities)
		{
			Image *image = new Image("assets/images/circle.png");
			image->resize(0.05);
			image->setLeft(0, top);
			glm::vec3 colour = colours->colour_for(ent->name());
			image->setColour(colour.x, colour.y, colour.z);
			box->addObject(image);

			Text *text = new Text(ent->name());
			text->setRight(-0.01, top);
			box->addObject(text);
			
			top += 0.07;
		}
		return box;
	};
	
	auto show_topology = [model, parent, click_chain, show_legend,
	                      provide_colours]()
	{
		if (model->filename().length() == 0)
		{
			BadChoice *bc = new BadChoice(parent, 
			                              "Error: filename missing");
			parent->setModal(bc);
		}
		else
		{
			ModelTopologyView *mtv = 
			new ModelTopologyView(parent, *model);
			mtv->setClickChainEvent(click_chain);
			mtv->setColouringFunction(provide_colours);
			mtv->setLegendFunction(show_legend);
			mtv->show();
		}
	};

	return show_topology;
}
