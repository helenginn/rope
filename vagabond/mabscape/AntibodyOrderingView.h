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

#ifndef __vagabond__AntibodyOrderingView__
#define __vagabond__AntibodyOrderingView__

#include "MultipleSetup.h"
#include <vagabond/utils/svd/PCA.h>
#include <mutex>

struct Mab;
class MatrixBox;
class MatrixPlot;
class TextButton;
struct Competition;

class AntibodyOrderingView : public MultipleSetup<Competition>
{
public:
	AntibodyOrderingView(Scene *prev, Mab &mab);

	virtual void setup();
	virtual void refresh();
	virtual void mouseReleaseEvent(double x, double y,
	                               SDL_MouseButtonEvent button);

	// when set, "Next" applies the new ordering, calls this and goes back to
	// the previous scene, rather than opening a new Playground
	void setReturnJob(const std::function<void()> &job)
	{
		_returnJob = job;
	}

	Competition &comp()
	{
		return *_object;
	}
protected:
	virtual bool acceptable_to_add_after(Competition &comp);
private:
	void makePlot();
	void addButtons();
	// the values of the plot again (inferred competition on or off), in the
	// order it has now, which is left as it is
	void updateValues();
	PCA::Matrix _forDisplay{};
	Mab &_mab;
	std::mutex _mutex;
	// right-click on a matrix element offers to put its row's antibody in
	// the naughty corner; each one there is listed to the right, with an
	// arrow to bring it back
	void rightClick(double x, double y);
	void putInNaughtyCorner(const std::string &name);
	void bringBack(const std::string &name);
	void listNaughtyCorner();
	// the order as it is now, so that nothing moves when the matrix is made
	// again with one more or less
	void keepCurrentOrder();
	std::vector<std::string> currentOrder();
	MatrixBox *_mb{};
	MatrixPlot *_plot{};
	std::vector<std::string> _baseOrder; // the order the plot was made in
	std::function<void()> _returnJob{};

};

#endif
