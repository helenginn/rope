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
#include <vagabond/gui/elements/ImageButton.h>
#include <vagabond/core/FastaFile.h>
#include <vagabond/core/Sequence.h>
#include <vagabond/core/Entity.h>
#include "SequenceView.h"
#include "AddEntity.h"
#include "FastaView.h"

FastaView::FastaView(Scene *prev, std::string filename, bool convertible) :
ListView(prev)
{
	_filename = filename;
	_convertible = convertible;
	_file = new FastaFile(_filename);
	_file->parse();
	
	_sequences = _file->namedSequences();
	for (auto it = _sequences.begin(); it != _sequences.end(); it++)
	{
		_names.push_back(it->first);
	}
}

FastaView::~FastaView()
{
	for (size_t i = 0; i < _toDel.size(); i++)
	{
		delete _toDel[i];
	}

	_toDel.clear();
}

void FastaView::setup()
{
	addTitle("Fasta file: " + _filename);
	ListView::setup();
}

size_t FastaView::lineCount()
{
	return _names.size();
}

Renderable *FastaView::getLine(int i)
{
	const std::string &id = _names[i];
	Box *b = new Box();
	Text *name = new Text(id);
	name->setLeft(0., 0.);
	b->addObject(name);
	const std::string &seq = _sequences[id];

	{
		Sequence *s = new Sequence(seq);
		TextButton *t = SequenceView::button(s, this);
		t->setReturnTag("sequence_" + id);
		t->setLeft(0.5, 0.0);
		b->addObject(t);
		_toDel.push_back(s);
	}
	
	if (_convertible)
	{
		ImageButton *t = ImageButton::arrow(-90., this);
		t->setReturnTag("choose_" + id);
		t->setCentre(0.7, 0);
		b->addObject(t);
	}
	
	return b;
}

void FastaView::buttonPressed(std::string tag, Button *button)
{
	ListView::buttonPressed(tag, button);
	
	std::string prefix = "sequence_";
	std::string end = Button::tagEnd(tag, prefix);
	if (end.length() > 0)
	{
		Sequence *full = new Sequence(_sequences[end]);

		SequenceView *view = new SequenceView(this, full);
		view->show();
		_toDel.push_back(full);
	}

	end = Button::tagEnd(tag, "choose_");
	if (end.length() > 0)
	{
		Scene *before = _previous->previous();
		AddEntity *addEntity = new AddEntity(this, _sequences[end]);
		addEntity->show();
	}
}
