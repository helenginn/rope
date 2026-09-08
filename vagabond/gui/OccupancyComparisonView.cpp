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

#include "OccupancyComparisonView.h"
#include "ChooseHeader.h"
#include "PerResidueView.h"
#include "VagWindow.h"

#include <vagabond/gui/elements/TextButton.h>
#include <vagabond/gui/elements/ImageButton.h>
#include <vagabond/gui/elements/Text.h>

#include <vagabond/core/Environment.h>
#include <vagabond/core/Entity.h>
#include <vagabond/core/ResidueTorsion.h>
#include <vagabond/core/Progressor.h>
#include <vagabond/core/RotamerOccupancy.h>
#include <vagabond/utils/DoJob.h>

OccupancyComparisonView::OccupancyComparisonView(Scene *prev, Entity *entity)
: Scene(prev), _entity(entity)
{
	_rota = std::make_shared<RotamerOccupancy>(entity);
}

OccupancyComparisonView::~OccupancyComparisonView()
{
	if (_cancelled)
	{
		_cancelled->store(true);
	}
}

void OccupancyComparisonView::setup()
{
	addTitle("Occupancy Comparison");

	addHeaderButton();
	addStubButtons();

	startMeasurement();
}

void OccupancyComparisonView::addHeaderButton()
{
	Text *t = new Text("Correlated metadata:");
	t->setLeft(0.15, 0.3);
	addObject(t);

	TextButton *b = new TextButton("Choose...", this);
	b->setReturnTag("header");
	b->setRight(0.85, 0.3);
	_headerButton = b;
	addObject(b);
}

void OccupancyComparisonView::refreshHeaderButton()
{
	std::string str = _header.length() ? _header : "Choose...";
	_headerButton->setText(str);
}

void OccupancyComparisonView::addStubButtons()
{
	{
		TextButton *t = new TextButton("Per-residue", this);
		t->setReturnTag("per_residue");
		t->setLeft(0.3, 0.6);
		t->setInert(true, true);
		_perResidueButton = t;
		addObject(t);

		ImageButton *arrow = ImageButton::arrow(-90, this);
		arrow->setReturnTag("per_residue");
		arrow->setCentre(0.7, 0.6);
		arrow->setInert(true, true);
		_perResidueArrow = arrow;
		addObject(arrow);
	}

	{
		TextButton *t = new TextButton("Pairwise correlation", this);
		t->setReturnTag("pairwise");
		t->setLeft(0.3, 0.68);
		t->setInert(true, true);
		_pairwiseButton = t;
		addObject(t);

		ImageButton *arrow = ImageButton::arrow(-90, this);
		arrow->setReturnTag("pairwise");
		arrow->setCentre(0.7, 0.68);
		arrow->setInert(true, true);
		_pairwiseArrow = arrow;
		addObject(arrow);
	}
}

void OccupancyComparisonView::startMeasurement()
{
	_group = _entity->makeTorsionDataGroup();
	_md = Environment::metadata();
	refreshHeaderButton();

	auto cancelled = std::make_shared<std::atomic<bool>>(false);
	_cancelled = cancelled;

	struct MeasureProgress : public Progressor {};
	MeasureProgress *progress = new MeasureProgress();

	auto cancelJob = [cancelled]()
	{
		cancelled->store(true);
	};

	int steps = (int)_entity->instances().size();

	VagWindow::window()->requestProgressBar(steps, "Measuring torsion angles",
	                                        progress, cancelJob);

	std::shared_ptr<RotamerOccupancy> rota = _rota;
	std::vector<ResidueTorsion> headers = _group.headers();

	auto measure = [this, rota, progress, cancelled, headers]()
	{
		rota->calculate(headers, progress, cancelled.get());

		VagWindow::window()->addMainThreadJob(
		[this, progress, cancelled]()
		{
			delete progress;

			if (cancelled->load())
			{
				return;
			}

			_perResidueButton->setInert(false, true);
			_perResidueArrow->setInert(false, true);
			_pairwiseButton->setInert(false, true);
			_pairwiseArrow->setInert(false, true);
		});
	};

	new DoJob(measure);
}

void OccupancyComparisonView::buttonPressed(std::string tag, Button *button)
{
	if (tag == "header")
	{
		ChooseHeader *ch = new ChooseHeader(this);
		ch->setResponder(this);
		ch->setData(_md, &_group);
		ch->show();
	}
	else if (tag == "per_residue")
	{
		PerResidueView *view = new PerResidueView(this, _entity, _rota,
		                                          _md, _header);
		view->show();
	}

	Scene::buttonPressed(tag, button);
}

void OccupancyComparisonView::sendObject(std::string header, void *object)
{
	_header = header;
	refreshHeaderButton();
}
