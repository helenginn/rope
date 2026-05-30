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

#include "PosToComp.h"
#include <vagabond/gui/MatrixPlot.h>
#include "Mab.h"
#include "Positions.h"

PosToComp::PosToComp(Competition &comp, Positions &pos) 
: _comp(comp), _positions(pos)
{
	_data = _comp.make_plot(_comp.favoured_ordering);
	_model = Eigen::MatrixXf(_data);
	_model.setZero();

	_dataDisplay = PCA::Matrix(_data);
	_modelDisplay = PCA::Matrix(_model);
}

Eigen::MatrixXf PosToComp::fromModel(MatrixPlot *mp)
{
	auto convert = [this](const float &f)
	{
		float inflection = _radius * 2;
		float exponent = exp((inflection - f) / _slope);
		float val = exponent / (1 + exponent);
		return val;
	};

	auto &names = _comp.favoured_ordering;
	int n = 0; int m = 0;
	for (auto it = names.begin(); it != names.end(); it++)
	{
		const std::string &left = *it;
		const glm::vec3 &lpos = _positions(left);

		n = m;
		for (auto jt = it + 1; jt != names.end(); jt++)
		{
			const std::string &right = *jt;
			const glm::vec3 &rpos = _positions(right, &lpos);

			float dist = glm::length(rpos - lpos);
			float score = convert(dist);
			
			_model(m, n) = score;
			_model(n, m) = score;
			
			n++;
		}
		
		m++;
	}
	
	if (mp)
	{
		{
			std::unique_lock<std::mutex> lock(_modelMutex);
			_modelDisplay.dropFromEigen(_model);
		}
		mp->update();
	}

	return _model;
}


