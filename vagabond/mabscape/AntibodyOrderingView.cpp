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

#include "AntibodyOrderingView.h"
#include "Playground.h"
#include <vagabond/gui/MatrixBox.h>
#include <vagabond/gui/MatrixPlot.h>
#include <vagabond/gui/elements/TickBoxes.h>
#include "Mab.h"

AntibodyOrderingView::AntibodyOrderingView(Scene *prev, Mab &mab)
: MultipleSetup(prev, mab.competitions), _mab(mab)
{

}

void AntibodyOrderingView::setup()
{
	addTitle("Adjust antibody ordering");
	
	refresh();
	addButtons();
}

void AntibodyOrderingView::makePlot()
{
	std::vector<std::string> order = comp().favoured_ordering;
	bool reorder = (order.size() == 0);

	Eigen::MatrixXf mat = comp().make_plot(order);

	_forDisplay = PCA::Matrix(mat);

	MatrixPlot *plot = new MatrixPlot(_forDisplay);
	plot->resize(1.5);
	MatrixBox *mb = new MatrixBox(plot, order, order, reorder, this);
	mb->setCentre(0.5, 0.6);
	addTempObject(mb);
	_mb = mb;
	_baseOrder = order;
}

void AntibodyOrderingView::addButtons()
{
	auto save_and_move_on = [this]()
	{
		std::vector<std::string> reordered = _mb->rowNames();
		comp().favoured_ordering = reordered;

		if (_returnJob)
		{
			_returnJob();
			back();
			return;
		}

		Playground *playground = new Playground(this, _mab);
		playground->show();
	};
	
	auto save_json = [this]()
	{
		std::vector<std::string> reordered = _mb->rowNames();
		comp().favoured_ordering = reordered;
		_mab.save();

		// shown again from the playground, with no Next: saving is how the
		// new ordering reaches it
		if (_returnJob)
		{
			_returnJob();
		}
	};

	{
		TextButton *tb = new TextButton("Save");
		tb->setRight(0.9, 0.1);
		tb->setReturnJob(save_json);
		addObject(tb);
	}

	// Next goes on to a playground; when this was shown from one already
	// there is nowhere to go on to
	if (!_returnJob)
	{
		TextButton *tb = new TextButton("Next");
		tb->setRight(0.9, 0.9);
		tb->setReturnJob(save_and_move_on);
		addObject(tb);
	}

	// Competition inferred from the rest of the data, kept apart from it, in
	// the bottom right corner
	{
		const std::string text = "infer competition";
		TickBoxes *tb = new TickBoxes(this, this);
		tb->setOneOnly(false);
		tb->addOption(text, [this, tb, text]()
		{
			bool on = tb->isTicked(text);

			if (on)
			{
				size_t made = comp().infer();
				std::cout << "Inferred " << made << " competition values" 
				<< std::endl;
			}

			comp().useInferred = on;
			updateValues();

			// shown again from the playground: its data matrix and its fit
			// have to be made again from this, as saving would do
			if (_returnJob)
			{
				_returnJob();
			}
		}, comp().useInferred);
		tb->arrange(0.74, 0.945, 0.98, 0.99);
		addObject(tb);
	}
}

void AntibodyOrderingView::updateValues()
{
	if (!_mb)
	{
		return;
	}

	// the same rows and columns as it was made with, whatever the order they
	// are shown in now, so that nothing moves
	std::vector<std::string> order = _baseOrder;
	Eigen::MatrixXf mat = comp().make_plot(order);
	_mb->setOriginalMatrix(mat);
}

void AntibodyOrderingView::refresh()
{
	deleteTemps();
	makePlot();

}

bool AntibodyOrderingView::acceptable_to_add_after(Competition &comp)
{
	return false;
}
