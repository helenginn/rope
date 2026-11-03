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
#include "Positions.h"
#include <vagabond/gui/MatrixBox.h>
#include <vagabond/gui/MatrixPlot.h>
#include <vagabond/gui/elements/TickBoxes.h>
#include <vagabond/gui/elements/Menu.h>
#include <vagabond/gui/elements/Text.h>
#include <vagabond/gui/elements/ImageButton.h>
#include <algorithm>
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
	_plot = plot;
	_baseOrder = order;

	listNaughtyCorner();
}

// The order on show, top to bottom. The matrix box lists its rows from the
// bottom up (rowNames()), so that list is not what makes the same matrix
// again: its columns, left to right, are.
std::vector<std::string> AntibodyOrderingView::currentOrder()
{
	return _mb->colNames();
}

void AntibodyOrderingView::keepCurrentOrder()
{
	if (_mb)
	{
		comp().favoured_ordering = currentOrder();
	}
}

void AntibodyOrderingView::mouseReleaseEvent(double x, double y,
                                             SDL_MouseButtonEvent button)
{
	Scene::mouseReleaseEvent(x, y, button);

	if (button.button == SDL_BUTTON_RIGHT)
	{
		rightClick(x, y);
	}
}

void AntibodyOrderingView::rightClick(double x, double y)
{
	if (!_mb || !_plot)
	{
		return;
	}

	convertToGLCoords(&x, &y);
	std::vector<std::string> rows = _mb->rowNames();
	std::string name;

	float cx, cy;
	_plot->fractionalPos(x, y, cx, cy);
	if (cx >= 0 && cx < 1 && cy >= 0 && cy < 1)
	{
		// the antibody of the row clicked on, counting rows from the bottom
		// as the matrix box does
		int row = std::min((int)rows.size() - 1, (int)(cy * rows.size()));
		if (row >= 0)
		{
			name = rows[row];
		}
	}
	else
	{
		// or the label on a side which was clicked on: its text is the name
		Text *label = dynamic_cast<Text *>(findObject(x, y));
		if (label && std::find(rows.begin(), rows.end(), label->text()) 
		    != rows.end())
		{
			name = label->text();
		}
	}

	if (name.length() == 0)
	{
		return;
	}

	Menu *menu = new Menu(this);
	menu->addOption("put in naughty corner", [this, name]()
	{
		putInNaughtyCorner(name);
	});

	float fx, fy;
	getFractionalPos(fx, fy);
	menu->setup(fx, fy, 1);
	setModal(menu);
}

void AntibodyOrderingView::putInNaughtyCorner(const std::string &name)
{
	keepCurrentOrder();
	auto &order = comp().favoured_ordering;
	order.erase(std::remove(order.begin(), order.end(), name), order.end());

	auto &corner = comp().naughty_corner;
	if (std::find(corner.begin(), corner.end(), name) == corner.end())
	{
		corner.push_back(name);
	}

	Positions::setIgnored(corner);

	// shown from the playground: its data matrix and its fit are made again
	if (_returnJob)
	{
		_returnJob();
	}

	refresh();
}

void AntibodyOrderingView::bringBack(const std::string &name)
{
	keepCurrentOrder();
	auto &corner = comp().naughty_corner;
	corner.erase(std::remove(corner.begin(), corner.end(), name), corner.end());

	Positions::setIgnored(corner);

	// back in at the end of the matrix
	if (_returnJob)
	{
		_returnJob();
	}

	refresh();
}

void AntibodyOrderingView::listNaughtyCorner()
{
	const std::vector<std::string> &corner = comp().naughty_corner;
	if (corner.empty())
	{
		return;
	}

	// squashed if the list would run off the bottom of the page
	const float top = 0.3;
	const float bottom = 0.85;
	float scale = std::min(1.f, (bottom - top) / (0.05f * corner.size()));

	Text *title = new Text("Naughty corner");
	title->resize(0.6);
	title->setLeft(0.78, top - 0.06);
	addTempObject(title);

	float y = top;
	for (const std::string &name : corner)
	{
		// the arrow points back at the matrix
		ImageButton *arrow = ImageButton::arrow(+90., this);
		arrow->setCentre(0.80, y);
		arrow->setReturnJob([this, name]() { bringBack(name); });
		addTempObject(arrow);

		Text *text = new Text(name);
		text->resize(0.6 * scale);
		text->setLeft(0.83, y);
		addTempObject(text);

		y += 0.05 * scale;
	}
}

void AntibodyOrderingView::addButtons()
{
	auto save_and_move_on = [this]()
	{
		std::vector<std::string> reordered = currentOrder();
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
		std::vector<std::string> reordered = currentOrder();
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
