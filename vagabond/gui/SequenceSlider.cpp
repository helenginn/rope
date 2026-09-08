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

#include "SequenceSlider.h"
#include <vagabond/gui/elements/ScrollBox.h>
#include <vagabond/gui/elements/Slider.h>
#include <vagabond/gui/elements/TextButton.h>
#include <vagabond/gui/elements/Text.h>
#include <vagabond/core/IndexedSequence.h>
#include <vagabond/core/Residue.h>

#include <algorithm>

const double SequenceSliderSpacing = 0.02;
// non-zero so tick Text's top half doesn't clip against the ScrollBox edge
const double SequenceSliderTickY = 0.02;
const double SequenceSliderRowY = SequenceSliderTickY + 0.05;
// keeps first/last residue buttons from poking past the viewport edge
const double SequenceSliderMarginX = SequenceSliderSpacing / 2;

SequenceSlider::SequenceSlider(IndexedSequence *sequence) : _sequence(sequence)
{

}

void SequenceSlider::setBounds(double left, double top, double right,
                                double bottom, double sliderY)
{
	_left = left;
	_top = top;
	_right = right;
	_bottom = bottom;
	_sliderY = sliderY;
}

void SequenceSlider::setup()
{
	buildContent();

	_scroll = new ScrollBox();
	_scroll->setContent(_content);
	_scroll->setBounds(glm::vec4(_top, _left, _bottom, _right));
	addObject(_scroll);

	double viewportWidth = _right - _left;
	double contentWidth = SequenceSliderSpacing * _sequence->entryCount()
	                     + SequenceSliderMarginX * 2;
	// always present, even with nothing to scroll, so a short sequence
	// still shows a slider rather than silently omitting it
	double maxScroll = std::max(contentWidth - viewportWidth,
	                             SequenceSliderSpacing);

	Slider *slider = new Slider();
	slider->setDragResponder(this);
	slider->resize(viewportWidth);
	slider->setup("", 0.0, maxScroll, maxScroll / 2000., false);
	slider->setStart(0., 0.);
	slider->setCentre((_left + _right) / 2., _sliderY);
	addObject(slider);
	_slider = slider;
}

void SequenceSlider::buildContent()
{
	_content = new Box();

	double x = SequenceSliderMarginX;
	size_t count = _sequence->entryCount();

	for (size_t i = 0; i < count; i++)
	{
		std::string str = _sequence->displayString(0, i);

		TextButton *t = new TextButton(str, this);
		t->setReturnTag("residue");
		t->setCentre(x, SequenceSliderRowY);

		if (_sequence->hasResidue(0, i))
		{
			Residue *r = _sequence->residue(0, i);
			t->setReturnObject(r);
			_buttons[r] = t;

			int num = r->as_num();
			if (num % 10 == 0)
			{
				addTickLabel(num, x);
			}
		}

		_content->addObject(t);
		x += SequenceSliderSpacing;
	}

	_content->setLeft(_left, _top);
}

void SequenceSlider::addTickLabel(int number, double x)
{
	Text *t = new Text(std::to_string(number));
	t->resize(0.6);
	t->setCentre(x, SequenceSliderTickY);
	_content->addObject(t);
}

void SequenceSlider::repositionContent(double target)
{
	double delta = target - _scrollX;
	_content->addAlign(-delta, 0);
	_content->forceRender(true, false);
	_scrollX = target;
}

void SequenceSlider::finishedDragging(std::string tag, double x, double y)
{
	repositionContent(x);
}

void SequenceSlider::buttonPressed(std::string tag, Button *button)
{
	if (tag == "residue" && button != nullptr)
	{
		Residue *r = static_cast<Residue *>(button->returnObject());
		TextButton *t = static_cast<TextButton *>(button);

		if (_selectedButton != nullptr)
		{
			_selectedButton->setColour(1., 1., 1.);
		}

		t->setColour(1., 1., 0.);
		_selectedButton = t;
		_selected = r;

		if (_job)
		{
			_job(r);
		}
	}
}

void SequenceSlider::setEnabledResidues(const std::set<Residue *> &enabled)
{
	for (auto &pair : _buttons)
	{
		bool on = enabled.count(pair.first) > 0;
		pair.second->setInert(!on, true);
	}
}
