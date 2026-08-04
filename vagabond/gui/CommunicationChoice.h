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

#ifndef __vagabond__CommunicationChoice__
#define __vagabond__CommunicationChoice__

#include <vagabond/gui/elements/ListView.h>
#include <vagabond/utils/OpSet.h>

class Clique;
class Probe;

class CommunicationChoice : public ListView
{
public:
	CommunicationChoice(Scene *prev, Clique *clique);

	virtual void setup();
	virtual void refresh();

	virtual size_t lineCount();
	virtual Renderable *getLine(int i);
	virtual size_t unitsPerPage()
	{
		return 14;
	}
private:
	Clique *_clique{};
	void chooseReporters(bool includeWater);

	// recomputes _ordered from _candidates/current assignment state -
	// called by refresh(), so every assign/unassign (which all already
	// call refresh() themselves) picks it up.
	void updateOrder();

	// fixed at construction, never reordered afterwards: the canonical
	// "nothing assigned yet" order - atom alt-confs first, half H-bonds
	// second (see the constructor) - that unassigned signals fall back
	// to once unassigned again.
	std::vector<Probe *> _candidates;

	// display order, recomputed by updateOrder(): every currently-
	// assigned signal (in _candidates' own relative order), followed by
	// every unassigned one (ditto) - i.e. assigned signals move to the
	// front as a group, and settle back into their original relative
	// position among the unassigned ones the moment they're unassigned,
	// rather than e.g. wherever they'd fall if appended at the end.
	std::vector<Probe *> _ordered;

	// eye.png column-header label above the per-row watch tickboxes -
	// re-added every refresh() (mirrors TableView::displayHeaders()),
	// since ListView::refresh()/loadFilesFrom() deleteTemps()s everything
	// including this each time.
	void makeWatchHeader();
};

#endif
