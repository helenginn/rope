// vagabond
// Copyright (C) 2026 Helen Ginn
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

#ifndef __vagabond__SequenceSlider__
#define __vagabond__SequenceSlider__

#include <vagabond/gui/elements/Box.h>
#include <vagabond/gui/elements/ButtonResponder.h>
#include <vagabond/gui/elements/DragResponder.h>

#include <functional>
#include <map>
#include <set>

class IndexedSequence;
class Residue;
class ScrollBox;
class Slider;
class TextButton;

/** \class SequenceSlider a horizontally scrollable, single-line display of
 * an IndexedSequence: residue-number tick labels every 10 residues along
 * the top, clickable/highlightable residue buttons below, and a
 * horizontal slider beneath that pans the whole thing. Knows nothing
 * about which sequence it belongs to or why - the caller decides that by
 * choosing which IndexedSequence to hand it, and finds out which residue
 * was clicked via the job set with setReturnJob(). */

class SequenceSlider : public Box, public ButtonResponder,
public DragResponder
{
public:
	SequenceSlider(IndexedSequence *sequence);

	/** left/top/right/bottom set the visible (clipped) viewport in the
	 * usual full-screen-fraction convention; sliderY places the
	 * horizontal scrollbar beneath it. Deliberately NOT ScrollBox's own
	 * (y_min, x_min, y_max, x_max) argument order - call this instead of
	 * touching the ScrollBox directly. */
	void setBounds(double left, double top, double right, double bottom,
	                double sliderY);

	void setup();

	virtual void buttonPressed(std::string tag, Button *button = nullptr);
	virtual void finishedDragging(std::string tag, double x, double y);

	void setReturnJob(const std::function<void(Residue *)> &job)
	{
		_job = job;
	}

	Residue *selected()
	{
		return _selected;
	}

	/** residues not in this set are made inert (unclickable) - the caller
	 * decides what "enabled" means, this class just applies it visually */
	void setEnabledResidues(const std::set<Residue *> &enabled);
private:
	void buildContent();
	void addTickLabel(int number, double x);
	void repositionContent(double target);

	IndexedSequence *_sequence = nullptr;

	Box *_content = nullptr;
	ScrollBox *_scroll = nullptr;
	Slider *_slider = nullptr;

	Residue *_selected = nullptr;
	TextButton *_selectedButton = nullptr;
	std::function<void(Residue *)> _job;
	std::map<Residue *, TextButton *> _buttons;

	double _left = 0.1;
	double _top = 0.1;
	double _right = 0.9;
	double _bottom = 0.25;
	double _sliderY = 0.28;

	double _scrollX = 0;
};

#endif
