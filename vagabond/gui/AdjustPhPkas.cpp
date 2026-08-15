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

#include "AdjustPhPkas.h"
#include <vagabond/gui/elements/Slider.h>
#include <vagabond/gui/elements/Text.h>
#include <vagabond/gui/elements/ScrollBox.h>
#include <vagabond/gui/elements/list/LineGroup.h>
#include <vagabond/gui/elements/list/ItemLine.h>
#include <vagabond/core/protonic/Network.h>
#include <vagabond/core/Model.h>

namespace
{
	struct ResidueEntry
	{
		std::string code;
		std::string label;
	};

	// display order/labels as specified, not alphabetical - default
	// values themselves live in CustomProtonSettings's own constructor.
	// Histidine appears twice (HIS2/HIS1 - see CustomProtonSettings'
	// constructor for why) rather than once like every other entry.
	const std::vector<ResidueEntry> residueEntries = {
		{"ARG", "Arginine"},
		{"LYS", "Lysine"},
		{"CYS", "Cysteine"},
		{"HIS2", "Histidine (doubly protonated)"},
		{"HIS1", "Histidine (singly protonated)"},
		{"GLU", "Glutamic acid"},
		{"ASP", "Aspartic acid"},
		{"SER", "Serine"},
		{"THR", "Threonine"},
		{"TYR", "Tyrosine"},
	};
}

AdjustPhPkas::AdjustPhPkas(Scene *prev, Network &network)
: Scene(prev), _network(network)
{

}

void AdjustPhPkas::setup()
{
	addTitle("Adjust pH and pKas");

	_parent.setDisplayName("pH and pKas");

	Item *phItem = new Item();
	phItem->setDisplayName("Sample pH");
	phItem->setSelectable(true);
	phItem->setSelectJob([this, phItem](bool left)
	{
		if (!left) { return; }
		selectItem(phItem, "", "Sample pH");
	});
	_parent.addItem(phItem);

	Item *sideChains = new Item();
	sideChains->setDisplayName("amino acid side chains");
	_parent.addItem(sideChains);

	for (const ResidueEntry &re : residueEntries)
	{
		Item *item = new Item();
		item->setDisplayName(re.label);
		item->setSelectable(true);
		item->setSelectJob([this, item, re](bool left)
		{
			if (!left) { return; }
			selectItem(item, re.code, re.label);
		});
		sideChains->addItem(item);
	}

	LineGroup *lg = new LineGroup(&_parent, this);
	// same idiom as ViewCorrelations::makeList() - (x, 0.2) is the
	// top-level row's own left-edge anchor, independent of the
	// ScrollBox's own bounds below (see ScrollBox::setBounds()'s own
	// comment: its vec4 is (y_min, x_min, y_max, x_max), not
	// (left, top, right, bottom) - easy to misread). Kept in step with
	// the ScrollBox's own x_min just below, so the content shifts along
	// with the viewport rather than drifting relative to it.
	lg->setLeft(0.02, 0.2);
	_lg = lg;

	ScrollBox *sb = new ScrollBox();
	sb->setContent(lg);
	lg->setScrollBox(sb);
	sb->setBounds(glm::vec4(0.15, 0.02, 0.92, 0.37));
	addObject(sb);
	lg->refreshGroups();
	sb->addSliderIfNeeded();

	// sits along the left edge of the right-hand panel (just clear of
	// the list above, which now ends at x=0.37) rather than centred
	// over the slider - shows which entry is currently selected.
	_label = new Text("Sample pH");
	_label->setLeft(0.3, 0.2);
	addObject(_label);

	// caption for the slider itself - distinct from _label above (which
	// changes per selected entry) - text itself still varies between
	// "Default pH" and "Default pKa" depending on what's selected, see
	// selectItem().
	_pkaCaption = new Text("Default pH");
	_pkaCaption->setLeft(0.3, 0.35);
	addObject(_pkaCaption);

	_slider = new Slider();
	// resize() scales the track image's own current size, not a direct
	// screen fraction - 0.3 measured out to about 1/6 (~0.167) of screen
	// width, so this is a linear extrapolation (0.3 * 0.4/0.167) to
	// reach ~40%; likely needs a further nudge once seen running.
	_slider->resize(0.7);
	_slider->setup("", 0.0, 15.0, 0.1, true);
	// left edge in line with _label/_pkaCaption above.
	_slider->setLeft(0.3, 0.45);

	auto drag_me = [this](double x, double)
	{
		if (!_network.model())
		{
			return;
		}

		if (_currentCode.empty())
		{
			_network.model()->protonSettings().setPH((float)x);
		}
		else
		{
			_network.model()->protonSettings().setPKa(_currentCode, (float)x);
		}
	};
	_slider->setDragFunction(drag_me);
	addObject(_slider);

	selectItem(phItem, "", "Sample pH");
}

void AdjustPhPkas::selectItem(Item *item, const std::string &code,
                              const std::string &label)
{
	if (_selectedItem && _lg->display(_selectedItem))
	{
		_lg->display(_selectedItem)->setColour(0.0, 0.0, 0.0);
	}

	_selectedItem = item;
	if (_lg->display(_selectedItem))
	{
		_lg->display(_selectedItem)->setColour(0.4, 0.0, 0.4);
	}

	_currentCode = code;
	_label->setText(label);
	_pkaCaption->setText(code.empty() ? "Default pH" : "Default pKa");

	if (!_network.model())
	{
		return;
	}

	CustomProtonSettings &settings = _network.model()->protonSettings();
	float current = code.empty() ? settings.pH() : settings.pKa(code);
	_slider->setStep(current);
}
