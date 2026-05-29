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
#include "Mab.h"

AntibodyOrderingView::AntibodyOrderingView(Scene *prev, Mab &mab)
: MultipleSetup(prev, mab.competitions), _mab(mab)
{

}

void AntibodyOrderingView::setup()
{
	addTitle("Adjust antibody ordering");
	
	refresh();
}

void AntibodyOrderingView::makePlot()
{
	std::vector<std::string> order;
	Eigen::MatrixXf mat = comp().make_plot(order);

	_forDisplay = PCA::Matrix(mat);

	MatrixPlot *plot = new MatrixPlot(_forDisplay);
	plot->resize(1.5);
	MatrixBox *mb = new MatrixBox(plot, order, order, true);
	mb->setCentre(0.5, 0.6);
	addTempObject(mb);

	auto save_and_move_on = [this, mb]()
	{
		std::cout << "Here" << std::endl;
		std::vector<std::string> reordered = mb->rowNames();
		comp().favoured_ordering = reordered;

		Playground *playground = new Playground(this, _mab);
		playground->show();
	};
	
	TextButton *tb = new TextButton("Next");
	tb->setRight(0.9, 0.9);
	tb->setReturnJob(save_and_move_on);
	addObject(tb);

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
