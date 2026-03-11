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

#include <vagabond/gui/elements/ImageButton.h>
#include <vagabond/gui/elements/AskForText.h>
#include <vagabond/gui/elements/TextButton.h>
#include <vagabond/gui/elements/TextEntry.h>
#include <vagabond/gui/elements/BadChoice.h>
#include <vagabond/gui/elements/Menu.h>
#include <vagabond/gui/ModelTopologyView.h>
#include <vagabond/gui/FileView.h>
#include <vagabond/core/SequenceComparison.h>
#include <vagabond/core/files/PdbFile.h>
#include <vagabond/core/AtomContent.h>
#include <vagabond/core/PolymerEntity.h>
#include <vagabond/core/EntityManager.h>
#include <vagabond/core/Chain.h>
#include "AntigenSetup.h"

AntigenSetup::AntigenSetup(Scene *scene, ColourMap &colours,
                           Antigens &antigens) 
: Scene(scene), _antigens(antigens), _colours(colours)
{
	if (_antigens.size() == 0)
	{
		_antigens.push_back(Antigen());
	}
	_antigen = _antigens.begin();
}

void AntigenSetup::prepareChoosePDB()
{
	auto select_pdb = [this](std::string filename)
	{
		try
		{
			PdbFile pdb(filename);
			pdb.parse();
			antigen().model.setFilename(filename);
			antigen().title = filename.substr(0, filename.rfind('.'));
			refresh();
		}
		catch (const std::runtime_error &err)
		{
			BadChoice *bc = new BadChoice(this, "Could not open antigen PDB: " 
			                              + std::string(err.what()));
			setModal(bc);
		}
	};
	
	auto choose_pdb = [this, select_pdb]()
	{
		FileView *fv = new FileView(this, select_pdb);
		if (fv->lineCount() == 0)
		{
			fv->globRefresh();
		}
		fv->show();
	};

	std::string name = antigen().model.filename().length() 
	? antigen().model.filename() : "choose";
	TextButton *tb = new TextButton(name, this);
	tb->setRight(0.8, 0.3);
	tb->setReturnJob(choose_pdb);
	addTempObject(tb);
}

void AntigenSetup::prepareChooseTitle()
{
	if (antigen().title.length() == 0)
	{
		return;
	}

	Text *text = new Text("Name of antigen:");
	text->setLeft(0.2, 0.4);
	addTempObject(text);
	
	TextEntry *te = new TextEntry(antigen().title, this);
	auto change_title = [this, te]()
	{
		antigen().title = te->scratch();
	};

	te->setRight(0.8, 0.4);
	te->setReturnJob(change_title);
	addTempObject(te);
}

void AntigenSetup::prepareAssignChains()
{
	if (antigen().model.filename().length() == 0)
	{
		return;
	}

	Text *text = new Text("Assign chains");
	text->setLeft(0.2, 0.5);
	addTempObject(text);
	
	auto provide_colours = [this](Chain *ch) -> glm::vec3
	{
		std::string entity = antigen().model.entityForChain(ch->id());
		if (entity.length() == 0)
		{
			return {0., 0., 0.};
		}
		else
		{
			return _colours.colour_for(entity);
		}
	};

	auto assign_new_entity = [this](Chain *ch, ModelTopologyView *view)
	{
		return [ch, view, this](std::string str)
		{
			try
			{
				PolymerEntity obj; obj.setName(str);
				obj.setSequence(ch->fullSequence()); 
				Environment::entityManager()->insertIfUnique(obj);
				antigen().model.setEntityForChain(ch->id(), str);
				antigen().model.housekeeping();
				_colours.recalculate();
				antigen().entities += str;
				view->updateColours();
			}
			catch (const std::runtime_error &err)
			{
				BadChoice *bad = new BadChoice(this, err.what());
				view->setModal(bad);
			}
		};
	};

	auto assign_monomer = [this]
	(ModelTopologyView *view, Chain *ch, const std::string &ent)
	{
		return [ent, ch, view, this]()
		{
			antigen().model.setEntityForChain(ch->id(), ent);
			antigen().model.housekeeping();
			_colours.recalculate();
			antigen().entities += ent;
			view->updateColours();
		};
	};

	auto unassign_monomer = [this]
	(ModelTopologyView *view, Chain *ch)
	{
		return [ch, view, this]()
		{
			antigen().model.unassignChainEntity(ch->id());
			antigen().model.housekeeping();
			_colours.recalculate();
			view->updateColours();
		};
	};

	auto make_ask_for_monomer = [assign_new_entity]
	(ModelTopologyView *view, Chain *ch)
	{
		return [assign_new_entity, ch, view]()
		{
			AskForText *aft = 
			new AskForText(view, "Name new antigen monomer:", 
			               "", nullptr);
			aft->setReturnJob(assign_new_entity(ch, view));
			view->setModal(aft);
		};
	};

	auto click_chain = [make_ask_for_monomer, assign_monomer, 
	unassign_monomer, this]
	(ModelTopologyView *view, Chain *ch, const glm::vec3 &where)
	{
		EntityManager *m = Environment::entityManager();
		std::vector<Entity *> entities = m->entities();

		Menu *menu = new Menu(view);
		menu->addOption("assign as new antigen protein",
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
		if (antigen().model.entityForChain(ch->id()).length())
		{
			menu->addOption("unassign", unassign_monomer(view, ch));

		}
		menu->setup(where.x, where.y, 1);
		view->setModal(menu);
	};
	
	auto show_topology = [this, click_chain, provide_colours]()
	{
		if (antigen().model.filename().length() == 0)
		{
			BadChoice *bc = new BadChoice(this, "Error: filename missing");
			setModal(bc);
		}
		else
		{
			ModelTopologyView *mtv = 
			new ModelTopologyView(this, antigen().model);
			mtv->setClickChainEvent(click_chain);
			mtv->setColouringFunction(provide_colours);
			mtv->show();
		}
	};

	ImageButton *t = ImageButton::arrow(-90., this);
	t->setReturnJob(show_topology);
	t->setCentre(0.8, 0.5);
	addTempObject(t);
}

void AntigenSetup::setup()
{
	addTitle("Antigen details");

	Text *text = new Text("Antigen-only PDB file");
	text->setLeft(0.2, 0.3);
	addObject(text);
	
	refresh();
}

void AntigenSetup::refresh()
{
	deleteTemps();
	prepareChoosePDB();
	prepareChooseTitle();
	prepareAssignChains();
	listAntigenChains();
	scrollButtons();
	deleteButton();
}

void AntigenSetup::deleteButton()
{
	if (_antigens.size() > 1)
	{
		TextButton *tb = new TextButton("Delete", this);
		tb->setRight(0.9, 0.1);
		tb->setReturnJob
		([this]()
		 {
			_antigens.erase(_antigen);
			_antigen = _antigens.begin();
			refresh();
		 });
		addTempObject(tb);
	}

}

void AntigenSetup::listAntigenChains()
{
	if (antigen().entities.size() == 0)
	{
		return;
	}

	Text *text = new Text("Antigen entities:");
	text->setLeft(0.2, 0.6);
	addTempObject(text);

	float top = 0.6;
	for (const std::string &ent : antigen().entities)
	{
		Text *text = new Text(ent);
		text->setRight(0.8, top);
		addTempObject(text);

		ImageButton *ib = new ImageButton("assets/images/cross.png", 
		                                  this);
		ib->resize(0.06);
		ib->setLeft(0.8, top);
		ib->setReturnJob
		([this, ent]()
		 {
			antigen().entities -= ent;
			refresh();
		 });
		addTempObject(ib);

		top += 0.06;
	}
}

void AntigenSetup::scrollButtons()
{
	if (_antigen != _antigens.begin())
	{
		ImageButton *bb = ImageButton::arrow(+90., this);
		bb->setCentre(0.1, 0.8);
		bb->setReturnJob
		([this]()
		 {
			_antigen--;
			refresh();
		 });
		addTempObject(bb);
	}

	if (_antigen != _antigens.end() - 1)
	{
		ImageButton *bb = ImageButton::arrow(-90., this);
		bb->setCentre(0.9, 0.8);
		bb->setReturnJob
		([this]()
		 {
			_antigen++;
			refresh();
		 });
		addTempObject(bb);
	}

	if (_antigen == _antigens.end() - 1 && 
	    antigen().model.filename().length() > 0)
	{
		ImageButton *bb = new ImageButton("assets/images/plus.png", 
		                                  this);
		bb->resize(0.06);
		bb->setCentre(0.9, 0.8);
		bb->setReturnJob
		([this]()
		 {
			_antigens.push_back(Antigen());
			_antigen = _antigens.end() - 1;
			refresh();
		 });
		addTempObject(bb);
	}
}
