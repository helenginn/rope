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

#ifndef __vagabond__PosToComp__
#define __vagabond__PosToComp__

#include <vagabond/utils/Eigen/Dense>
#include <vagabond/utils/svd/PCA.h>
#include <vagabond/core/Engine.h>

struct Competition;

class Positions;
class MatrixPlot;

class PosToComp : public RunsEngine
{
public:
	PosToComp(Competition &comp, Positions &pos);
	
	void setPlot(MatrixPlot *mp)
	{
		_plot = mp;
	}

	Eigen::MatrixXf &fromModel();

	PCA::Matrix &modelDisplay(std::mutex **mut)
	{
		*mut = &_modelMutex;
		return _modelDisplay;
	}

	PCA::Matrix &dataDisplay(std::mutex **mut)
	{
		*mut = &_dataMutex;
		return _dataDisplay;
	}

	void refine();
	float score();
private:
	void refineEngine();
	int sendJob(const std::vector<float> &all, Engine *sender);
	void zeroSetter();

	Competition &_comp;
	Positions &_positions;

	Eigen::MatrixXf _data;
	Eigen::MatrixXf _model;
	
	PCA::Matrix _dataDisplay;
	PCA::Matrix _modelDisplay;
	std::mutex _modelMutex;
	std::mutex _dataMutex;
	
	MatrixPlot *_plot{};
	
	float _radius{11.f};
	float _slope{2.f};
	
	std::function<void(const std::vector<float> &, bool)> _setter{};
	std::function<float(int n, int coord)> _gradient_for{};
};

#endif
