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

#include "CommunicationChoice.h"
#include <vagabond/gui/elements/TextButton.h>
#include <vagabond/gui/elements/ImageButton.h>
#include <vagabond/gui/elements/AskYesNo.h>
#include <vagabond/gui/elements/AskForText.h>
#include <vagabond/core/protonic/Clique.h>
#include <vagabond/utils/FileReader.h>
#include <cctype>

CommunicationChoice::CommunicationChoice(Scene *prev, Clique *clique)
: ListView(prev), _clique(clique)
{
	_candidates = clique->nonWaterProbes().toVector();

}

namespace
{
	// "A-Asn65" - chain, title-cased three-letter code, residue number -
	// short enough to read cleanly as an axis label in the matrix plot,
	// unlike the full probe desc ("A-ASN65:CA,A").
	std::string short_residue_name(Atom *atom)
	{
		std::string code = atom->code();
		to_lower(code);
		if (code.size())
		{
			code[0] = std::toupper((unsigned char)code[0]);
		}

		std::string name;
		if (atom->chain().length())
		{
			name += atom->chain() + "-";
		}
		name += code + i_to_str(atom->residueNumber());
		return name;
	}
}

void CommunicationChoice::setup()
{
	addTitle("Choose signals");

	TextButton *tb = new TextButton("Automatic");
	tb->setRight(0.9, 0.1);
	tb->setReturnJob([this]()
	{
		AskYesNo *ayn = new AskYesNo(this, "Include waters as signals?");
		ayn->addJob("yes", [this]() { chooseReporters(true); refresh(); });
		ayn->addJob("no", [this]() { chooseReporters(false); refresh(); });
		setModal(ayn);
	});
	addObject(tb);

	TextButton *clear = new TextButton("Clear all");
	clear->setRight(0.9, 0.2);
	clear->setReturnJob([this]()
	{
		_clique->clearCommunicationPoints();
		refresh();
	});
	addObject(clear);

	ListView::setup();
}

void CommunicationChoice::refresh()
{
	ListView::refresh();
}

size_t CommunicationChoice::lineCount()
{
	return _candidates.size();
}

Renderable *CommunicationChoice::getLine(int i)
{
	Box *box = new Box();

	Probe *probe = _candidates[i];
	std::string desc = probe->desc();
	std::string group = _clique->groupOfNode(desc);

	if (group.length())
	{
		Text *t = new Text(desc);
		t->setLeft(0., 0.);
		t->resize(0.6);
		box->addObject(t);

		Text *name = new Text(group);
		name->setLeft(0.5, 0.);
		name->resize(0.6);
		box->addObject(name);

		auto clear_one = [this, desc]()
		{
			_clique->removeCommunicationPoints({desc});
			refresh();
		};

		ImageButton *ib = new ImageButton("assets/images/cross.png", this);
		ib->resize(0.06);
		ib->setRight(1.0, 0.);
		ib->setReturnJob(clear_one);
		box->addObject(ib);
	}
	else
	{
		// abandoned: a "group" used to be able to hold several signals at
		// once, but that turned out not to be useful - each row here now
		// becomes exactly one named signal (one probe, one display name).
		auto make_signal = [this, desc]()
		{
			AskForText *aft = new AskForText(this, "Display name for signal:",
			                                 "", this);
			aft->setReturnJob([this, desc](std::string name)
			{
				if (name.length())
				{
					_clique->addCommunicationPoints(name, {desc});
				}
				refresh();
			});
			setModal(aft);
		};

		TextButton *tb = new TextButton(desc, this);
		tb->setLeft(0., 0.);
		tb->resize(0.6);
		tb->setReturnJob(make_signal);
		box->addObject(tb);
	}

	return box;
}

void CommunicationChoice::chooseReporters(bool includeWater)
{
	OpSet<std::pair<std::string, ResidueId>> done;

	auto is_water = [](Probe *probe)
	{
		return probe->atom()->code() == "HOH";
	};

	for (Probe *probe : _candidates)
	{
		if (probe->is_atom() && probe->atom()->isReporterAtom())
		{
			// bulk water is a synthetic pseudo-atom standing in for the
			// disordered/liberated fraction of a partially-occupied water
			// (see Network.cpp's setBulk(true)) - it has no real position
			// of its own, so it should never be picked as a signal, even
			// when explicit (fully real) waters are allowed.
			if (probe->is_bulk())
			{
				continue;
			}

			if (!includeWater && is_water(probe))
			{
				continue;
			}

			std::pair<std::string, ResidueId> id =
			{probe->atom()->chain(), probe->atom()->residueId()};

			if (done.count(id) == 0)
			{
				_clique->addCommunicationPoints(
				short_residue_name(probe->atom()), probe->desc());
				done += id;
			}
		}
	}

	for (Probe *probe : _candidates)
	{
		if (probe->is_atom())
		{
			if (probe->is_bulk())
			{
				continue;
			}

			if (!includeWater && is_water(probe))
			{
				continue;
			}

			std::pair<std::string, ResidueId> id =
			{probe->atom()->chain(), probe->atom()->residueId()};

			if (done.count(id) == 0)
			{
				_clique->addCommunicationPoints(
				short_residue_name(probe->atom()), probe->desc());
				done += id;
			}

		}
	}
}
